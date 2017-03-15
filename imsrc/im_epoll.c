/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include "im_epoll.h"
#include "im_ss.h"
#include "pt.h"
#include "im_rbtimer.h"

extern server_status_t *p_imserver; //server 状态

static struct im_epoll_s imepoll; //epoll事件驱动机制
extern ngx_queue_t hooksign_l, hooklogin_l, hookalive_l, hookcpush_l, hooklogout_l, hookspush_l;

ngx_int_t im_epoll_init()
{
	//初始化定时器事件
	update_imtime();
	ak_event_timer_init();

	if(init_imhook() != NGX_OK)
	{
		return NGX_ERROR;
	}

	ngx_memzero(&imepoll, sizeof(imepoll));
	imepoll.epfd = epoll_create(MAX_EPOLL_CONNECTION);
	if(imepoll.epfd == -1)
		return NGX_ERROR;


	return NGX_OK;
}

ngx_int_t ak_epoll_done()
{
	close(imepoll.epfd);
	return NGX_OK;
}

//当accept产生connect后将fd加入epoll
ngx_int_t im_epoll_add_connection(impush_conn_t *c)
{
	struct epoll_event ee;

	ee.events = EPOLLIN|EPOLLRDHUP; //不关注写事件,默认使用水平触发EPOLLLT
	ee.data.ptr = (void *) c;

	if(epoll_ctl(imepoll.epfd, EPOLL_CTL_ADD, c->fd, &ee) == -1)
		return NGX_ERROR;

	return NGX_OK;
}

ngx_int_t im_epoll_del_connection(impush_conn_t *c)
{
	struct epoll_event ee;

	ee.events = EPOLLIN|EPOLLET|EPOLLRDHUP; //不关注写事件
	ee.data.ptr = (void *) c;
	ee.data.fd = c->fd;

	if(epoll_ctl(imepoll.epfd, EPOLL_CTL_DEL, c->fd, &ee) == -1)
		return NGX_ERROR;

	return NGX_OK;
}

//该函数可能会阻塞超时
ngx_int_t im_epoll_process_events(ngx_msec_t timeout)
{
	int events;
	uint32_t revents;
	int i;
	impush_conn_t *c;
	ngx_int_t readret;
	struct timeval n_time;
	ngx_msec_t set_timeout;

	//预先查询定时器，设置最近的定时器触发时间差值给epoll_wait
	get_imtime(&n_time);
	set_timeout = ak_event_find_timer(n_time.tv_sec);
	if(set_timeout == NGX_TIMER_INFINITE)
	{
		set_timeout = timeout; //没有定时器事件
	}

	events = epoll_wait(imepoll.epfd, imepoll.epoll_evs, MAX_EPOLL_EVENTS, timeout);
	if(events == -1) //如果被信号中断，则优先执行信号处理函数，之后epoll_wait返回-1, errno被设置为EINTR
	{
		if(errno == EINTR)
		{
			//执行定时器事件
			update_imtime();
			get_imtime(&n_time);
			ak_event_expire_timers(n_time.tv_sec); //处理定时器事件

			return NGX_OK;
		}
		else
			return NGX_ERROR;
	}
	update_imtime();

	if(events == 0) //timeout
		return NGX_OK;

	
	for (i = 0; i < events; i++)
	{
		c = (impush_conn_t *) (imepoll.epoll_evs[i].data.ptr);
		if(c == NULL) //实际上不该发生
			continue;
		

		if(imepoll.epoll_evs[i].events & EPOLLRDHUP) //对方关闭套接字,对方关闭套接字时EPOLLRDHUP和EPOLLIN都会被置位(EPOLLIN被置位应该是为了兼容老版本epoll)，故先检查该标志
		{
			PT_Info("client:%u close fd!\n", c->id);
			c->imclose(c);
			continue;
		}
		else if(imepoll.epoll_evs[i].events & EPOLLIN)
		{
			switch(c->fdtype)
			{
				case 0:
				readret = c->imread(c);
				if( NGX_OK == readret )
				{	
					if( c->imhandle(c) == NGX_ERROR)
						c->imclose(c);
				}
				else if(NGX_ERROR == readret )
				{
					c->chatstatus = IMCHAT_WAITCLOSE;
					c->imclose(c);
				}
				break;

				case 1:
				c->accept_handle(c);
				break;
			}
		}
	}

	return NGX_OK;
}



int imconn_accept(impush_conn_t *c)
{
	int connfd;
	impush_conn_t *newc;
	struct sockaddr_in clientaddr;
	socklen_t client = sizeof(clientaddr);

	//这里注意可能同时有多个连接到来，而epoll只返回一次，所以应该循环调用accept将所有的连接都处理完
	while(1)
	{
		connfd = accept(c->fd, (struct sockaddr *)&clientaddr, &client);
		if(connfd < 0)
		{
			if(errno == EAGAIN)
			{
				break;  //从tcp就绪队列中取完了所有的连接
			}
			else
			{
				PT_Warn("listen fd accept failed\n");
				continue;
				//return -1;
			}
		}
		//看来每一个链接必须申请内存了
		newc = (impush_conn_t *)ngx_calloc(sizeof(impush_conn_t));
		if(newc == NULL)
			continue; //return 1;
		
		newc->fd = connfd;
		newc->sockaddr = clientaddr;
		newc->socklen = sizeof(clientaddr);
		newc->fdtype = 0; //客户端fd
		newc->imread = impush_read_data;
		newc->imhandle = impush_handle_data;
		newc->imclose = imconn_close;
		newc->chatstatus = IMCHAT_OPEN;
		newc->chat_pool = NULL;
		newc->in = NULL;
		newc->out = NULL;
		newc->dev = NULL;

		ngx_nonblocking(newc->fd);
		if(im_epoll_add_connection(newc) != NGX_OK)
		{
			PT_Warn("im_epoll_add_connection failed, errno:%d\n", errno);
			//im_epoll_del_connection(newc);
			close(connfd);
			ngx_free(newc);
			continue;//return -3;
		}
	}

	//init_conn_im_handle();

	return 0;
}

int imconn_close(impush_conn_t *c)
{
	im_epoll_del_connection(c);
	close(c->fd);
	ngx_destroy_pool(c->chat_pool);

	if(c->dev_associated == 1) //如果绑定了设备节点，登录操作成功就一定绑定了一个设备节点
	{
		PT_Info("close devid %u connection, unassociated addr:%x!\n", c->id, (int)(c->dev->connect) );
		c->dev->status = 0; //离线
		c->dev->connect = NULL;
		c->dev_associated = 0;
	}
	else
		PT_Warn("close connection, this connection not associated a dev!\n");

	c->dev = NULL;
	ngx_free(c);
}


static inline ngx_int_t create_chatio(impush_conn_t *c, impush_header_t *h)
{
	c->in = ngx_create_temp_buf(c->chat_pool, ntohs(h->len) + sizeof(impush_header_t) );
	if(c->in == NULL)
	{
		return NGX_ERROR;
	}
	
	c->out = ngx_create_temp_buf(c->chat_pool, IM_MSG_SIZE_LIMIT );
	if(c->out == NULL)
	{
		return NGX_ERROR;
	}

	c->chatting = 1;
	ngx_memcpy(c->in->last, (void *)h, sizeof(impush_header_t) );
	c->in->last += sizeof(impush_header_t);

	return NGX_OK;
}


ngx_int_t impush_read_data(impush_conn_t *c)
{
	impush_header_t im_h;
	uint16_t len;
	int readret;
	char *buf = (char *)&im_h;
	int i;

	if( read(c->fd, (void *)&im_h, sizeof(im_h)) != sizeof(im_h) )
	{
		PT_Warn("read info is too short!\n");
		//im_epoll_del_connection(c);
		//close(c->fd); //会自动从epfd事件中删除
		//ngx_free(c);
		c->chatstatus = IMCHAT_WRITING;
		imrespone_immediately(c->fd, 0, IMWARN_ERR);
		c->chatstatus = IMCHAT_WAITCLOSE;
		return NGX_ERROR;
	}

	len = ntohs(im_h.len); 
	if(len > IM_MSG_SIZE_LIMIT)
	{
		return NGX_DECLINED; //拒绝
	}
	c->chatstatus = IMCHAT_READING;

	if(c->chat_pool == NULL) //如果还没有创建内存池则此时创建
	{
		c->chat_pool = ngx_create_pool(4000);
		if(c->chat_pool == NULL)
		{
			imrespone_immediately(c->fd, im_h.session_id, IMWARN_ERR);
			return NGX_ERROR;
		}
		c->chatting = 0;
	}

	if(c->chatting == 0)
	{
		ngx_reset_pool(c->chat_pool);
		c->in = NULL;
		c->out = NULL;
	}
	else
	{
		imrespone_immediately(c->fd, im_h.session_id, IMWARN_BUSY);
		return NGX_BUSY;
	}

	if(c->in == NULL ) //每次会话开始
	{
		if(create_chatio(c, &im_h) == NGX_ERROR) //创建c->in,c->out并将报头复制进来
		{
			imrespone_immediately(c->fd, im_h.session_id, IMWARN_ERR);
			return NGX_ERROR;
		}
	}
	else if( c->in->pos == c->in->last)
	{
		if(create_chatio(c, &im_h) == NGX_ERROR)
		{
			imrespone_immediately(c->fd, im_h.session_id, IMWARN_ERR);
			return NGX_ERROR;
		}
	}

	if(len == 0)
		return NGX_OK;

	readret = read(c->fd, (void *)c->in->last, len);
	if( readret < 0 )
	{
		if(errno == EAGAIN)
		{
			return NGX_DONE;
		}
		else
		{
			imrespone_immediately(c->fd, im_h.session_id, IMWARN_ERR);
			return NGX_ERROR;
		}
	}
	else if(readret == len) //成功
	{
		c->in->last += readret;
		return NGX_OK;
	}
	else 
	{
		c->in->last += readret;
		return NGX_DONE;
	}
}


ngx_int_t impush_handle_data(impush_conn_t *c)
{
	impush_header_t *h = (impush_header_t *)(c->in->pos);
	ngx_int_t ret = NGX_OK;


	c->lasttime = time(NULL); //更新超时时间

	c->chatstatus = IMCHAT_HANDLING;
	switch(h->type)
	{
		case IMPUSH_SIGN:
			//ret = impush_sign(c);
			ret = runhook_queue(&hooksign_l, c);
		break;

		case IMPUSH_LOGIN:
			//ret = impush_login(c);
			ret = runhook_queue(&hooklogin_l, c);
		break;

		case IMPUSH_LOGOUT:
			//ret = impush_logout(c);
			ret = runhook_queue(&hooklogout_l, c);
		break;

		case IMPUSH_ALIVE:
			//ret = impush_alive(c);
			ret = runhook_queue(&hookalive_l, c);
		break;

		case IMPUSH_SPUSH:  //it is client respone data, ignore it!
			ret = runhook_queue(&hookspush_l, c);
		break;

		case IMPUSH_CPUSH:
			//ret = impush_cpush(c);
			ret = runhook_queue(&hookcpush_l, c);
		break;

		default:
		break;
	}

	if( ret == NGX_ERROR) //该函数返回NGX_ERROR即意味着下一步会close链接，释放资源
		return ret;

	c->chatstatus = IMCHAT_WRITING;
	//write back
	//if( (h->type != IMPUSH_SERACK) && (h->type != IMPUSH_CLIACK) )
	if(h->type % 2) //基数代表是处理客户端消息
		write(c->fd, c->out->pos, c->out->last - c->out->pos);

	c->chatting = 0;
	if(h->type == IMPUSH_LOGOUT)
	{ //close
		c->chatstatus = IMCHAT_WAITCLOSE;

	}
	else
		c->chatstatus = IMCHAT_KEEPING;

	return NGX_OK;
}
