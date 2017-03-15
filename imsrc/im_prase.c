/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include "im_prase.h"
#include "im_ss.h"
#include "pt.h"
#include "im_rbtimer.h"
#include "im_prase.h"

static uint32_t sequence_id = 100;

server_status_t imserver;
server_status_t *p_imserver = &imserver; //服务器端状态，可由此查看所有信息
//extern server_status_t *p_imserver;

//队列中按照优先级从高到低排列（值越小优先级越高）
ngx_queue_t hooksign_l, hooklogin_l, hookalive_l, hookcpush_l, hooklogout_l, hookspush_l;


impush_msg_t *im_publish_head = NULL;

//impush_t init_impush;


ngx_int_t init_env()
{
	ngx_queue_init(&hooksign_l);
	ngx_queue_init(&hooklogin_l);
	ngx_queue_init(&hookalive_l);
	ngx_queue_init(&hookcpush_l);
	ngx_queue_init(&hooklogout_l);
	ngx_queue_init(&hookspush_l);
}

ngx_int_t add_hook(ngx_queue_t *head, im_hook_t *r)
{
	ngx_queue_t *nq;
	im_hook_t *qnode;
	ngx_queue_init(&r->que);

	//遍历
	for(nq = ngx_queue_head(head); nq != ngx_queue_sentinel(head); nq = ngx_queue_next(nq))
	{
		qnode = ngx_queue_data(nq, im_hook_t, que);
		if(qnode->priorty < r->priorty)
		{
			continue; //继续搜索
		}
		else if(qnode->priorty > r->priorty)
		{
			ngx_queue_insert_after(nq, &r->que);
			return NGX_OK;
		}
		else //equal
			return NGX_ERROR;
	}

	ngx_queue_insert_tail(head, &r->que);
	return NGX_OK;
}

ngx_int_t hookregister(im_hook_t *r, int type)
{
	ngx_int_t ret;
	switch(type)
	{
		case IMPUSH_SIGN:
			ret = add_hook(&hooksign_l, r);
		break;

		case IMPUSH_LOGIN:
			ret = add_hook(&hooklogin_l, r);
		break;

		case IMPUSH_ALIVE:
			ret = add_hook(&hookalive_l, r);
		break;

		case IMPUSH_CPUSH:
			ret = add_hook(&hookcpush_l, r);
		break;	

		case IMPUSH_LOGOUT:
			ret = add_hook(&hooklogout_l, r);
		break;

		case IMPUSH_SPUSH:
			ret = add_hook(&hookspush_l, r);
		break;

		default:
			ret = NGX_ERROR;
		break;
	}

	return ret;
}

ngx_int_t runhook_queue(ngx_queue_t *head, impush_conn_t *c)
{
	ngx_queue_t *nq;
	im_hook_t *qnode;
	ngx_int_t ret = NGX_ERROR;

	for(nq = ngx_queue_head(head); nq != ngx_queue_sentinel(head); nq = ngx_queue_next(nq))
	{
		qnode = ngx_queue_data(nq, im_hook_t, que);
		if(qnode->hook == NULL)
			continue;
		
		ret = qnode->hook(c);
		if(ret == NGX_DONE)
			break;
		if(ret != NGX_OK)
			return ret;
	}

	return ret;
}

//添加预备发布的内容到服务器推送队列
ngx_int_t im_add_msg(int8_t *msgbuf, int16_t len)
{
	impush_msg_t *p;
	impush_msg_t *node = NULL;


	node = (impush_msg_t *)ngx_calloc(sizeof(impush_msg_t));
	if(node == NULL)
		return NGX_ERROR;

	node->next = NULL;
	node->len = len + sizeof(impush_header_t);
	node->buf = (uint8_t *)ngx_calloc(sizeof(impush_header_t) + len);
	if(node->buf == NULL)
	{
		ngx_free(node);
		return NGX_ERROR;
	}
	ngx_memcpy(node->buf + sizeof(impush_header_t), msgbuf, len);

	if(im_publish_head == NULL)
	{
		 im_publish_head = node;
		 return NGX_OK;
	}

	p = im_publish_head;
	while(p->next != NULL)
		p = p->next;

	 p->next = node;

	return NGX_OK;
}

void im_free_msg()
{
	impush_msg_t *p = im_publish_head;
	impush_msg_t *p1;

	im_publish_head = NULL;

	while(p != NULL)
	{
		p1 = p;
		p = p->next;

		ngx_free(p1->buf);
		ngx_free(p1);
	}

}


//服务器循环推送到每个客户端
//maxtimes:每个设备最多推送次数，每个
ngx_int_t impush_spush_pump()
{
	emdev_t *dev_m = NULL;
	impush_conn_t * c = NULL;
	impush_msg_t *p = im_publish_head;
	ngx_rbtree_node_t     *sen = &(p_imserver->rb_sentinel);
	ngx_rbtree_node_t *rbdev_min = NULL;
	impush_header_t *oh = NULL;
	int16_t tmp_id=0;

	if(im_publish_head == NULL)
		return NGX_OK;

	if(p_imserver->rb_dev.root == sen)
		return NGX_OK;

	rbdev_min = ngx_rbtree_min(p_imserver->rb_dev.root, sen);

	while(rbdev_min != NULL)
	{
		dev_m = rbtree_containerof(rbdev_min, emdev_t, r_devnode);
		//printf("spush , dev:%u, status:%d connect address:%x\n", dev_m->devid, dev_m->status, dev_m->connect);
		if(dev_m->status == 1 && dev_m->connect != NULL) //设备在线，链接有效
		{
			c = dev_m->connect;
			p = im_publish_head; //从头开始遍历
			while(p != NULL)
			{
				oh = (impush_header_t *)(p->buf);
				oh->ver = CURRENT_VER;
				oh->type = 10; 
				oh->warn = 0; //0
				oh->reserve = 0; 
				oh->len = htons(p->len - sizeof(impush_header_t)); 
				oh->session_id = htons(tmp_id++);

				PT_Info("push msg to dev:%d\n", c->id);
				write(c->fd, p->buf, p->len);
				p = p->next;
			}
		}

		rbdev_min = ngx_rbtree_next(rbdev_min, sen);
	}

	im_free_msg();
	return NGX_OK;
}

//清理超时设备连接
ngx_int_t impush_timeout_clear()
{
	emdev_t *dev_m = NULL;
	impush_conn_t * c = NULL;
	ngx_rbtree_node_t     *sen = &(p_imserver->rb_sentinel);
	ngx_rbtree_node_t *rbdev_min = NULL;
	time_t c_time;

	c_time = time(NULL);
	if(c_time > p_imserver->lasttimecheck + IMPUSH_ALIVE_TIMEOUT) //当前时间 超过 最近一次检测时间 + 超时限制时间
	{
		p_imserver->lasttimecheck = c_time;

		if(p_imserver->rb_dev.root == sen)
			return NGX_OK;
		
		rbdev_min = ngx_rbtree_min(p_imserver->rb_dev.root, sen);
		while(rbdev_min != NULL)
		{
			dev_m = rbtree_containerof(rbdev_min, emdev_t, r_devnode);
			if(dev_m->status == 1 && dev_m->connect != NULL) //设备在线，链接有效
			{
				c = dev_m->connect;
				if(c->lasttime + IMPUSH_ALIVE_TIMEOUT < c_time) //预期过期时间已经小于当前时间，超时
				{
					imconn_close(c);
					dev_m->connect = NULL;
				}
			}

			rbdev_min = ngx_rbtree_next(rbdev_min, sen);
		}	

	}
	return NGX_OK;
}



static inline void imdefault_header(impush_header_t *ih, impush_header_t *oh)
{
	oh->ver = CURRENT_VER;
	oh->type = ih->type; 
	oh->warn = 0; 
	oh->reserve = 0; 
	oh->len = htons(0); 
	oh->session_id = ih->session_id;
}

//不含实际报文内容的回复
int imrespone_immediately(int fd, int16_t n_id, uint8_t warn_type)
{
	impush_header_t imh;
	memset(&imh, 0, sizeof(impush_header_t));
	imh.ver = CURRENT_VER;
	//imh.type = 0;
	imh.warn = warn_type; 
	imh.reserve = 0;
	imh.len = htons(0);
	imh.session_id = n_id;

	return write(fd, (void *)&imh, sizeof(impush_header_t) );
}

/*
ngx_int_t impush_serverack(impush_conn_t *c)
{
	//更新时间

	return NGX_OK;
}

ngx_int_t impush_clientack(impush_conn_t *c)
{
	//更新时间

	return NGX_OK;
}
*/

ngx_int_t impush_login(impush_conn_t *c)
{
	impush_header_t *ih = (impush_header_t *)(c->in->pos);
	impush_header_t *oh = (impush_header_t *)(c->out->pos);
	uint32_t	*d_id = (uint32_t *)(ih + 1);
	emdev_t *dev = NULL;

	//init respone
	c->out->last += sizeof(impush_header_t);
	imdefault_header(ih, oh);

	dev = rbtree_find_dev(p_imserver, ntohl(*d_id) ); //检查是否已经存在
	if(dev == NULL)
	{
		oh->warn = IMWARN_NOSIGN;
		printf("login failed\n");
		imrespone_immediately(c->fd, ih->session_id, IMWARN_NOSIGN);
		return NGX_ERROR; //意味着服务器会主动关闭该链接
	}

	if(dev->connect != NULL)
	{
		imconn_close(dev->connect); //释放以前的资源
	}

	dev->status = 1; //上线
	dev->connect = c;
	get_imtime(&dev->last_time);
	dev->last_logintime = time(NULL);

	c->dev_associated = 1; // 链接绑定设备节点
	c->dev = dev;
	c->id = dev->devid;
	PT_Debug("the devid:%u login!, associate address:%x\n", dev->devid, (int)c);

	return NGX_OK;
}

ngx_int_t impush_logout(impush_conn_t *c)
{
	impush_header_t *ih = (impush_header_t *)(c->in->pos);
	impush_header_t *oh = (impush_header_t *)(c->out->pos);
	uint32_t	*d_id = (uint32_t *)(ih + 1);
	emdev_t *dev = NULL;

	//init respone
	c->out->last += sizeof(impush_header_t);
	imdefault_header(ih, oh);

	if(c->dev != NULL)
	{
		dev = c->dev;
	}
	else
	{
		dev = rbtree_find_dev(p_imserver, ntohl(*d_id) ); 
		if(dev == NULL)
		{
			oh->warn = IMWARN_NOSIGN;
			imrespone_immediately(c->fd, ih->session_id, IMWARN_NOSIGN);
			return NGX_ERROR;
		}
	}

	dev->status = 0; //离线
	dev->connect = NULL;
	get_imtime(&dev->last_time);

	c->dev_associated = 0; // 解锁绑定设备节点
	c->dev = NULL;
	c->chatting = 0;
	PT_Info("logout!\n");
	return NGX_OK;	
}

ngx_int_t impush_sign(impush_conn_t *c)
{
	impush_header_t *ih = (impush_header_t *)(c->in->pos);
	impush_header_t *oh = (impush_header_t *)(c->out->pos);
	uint64_t	info;
	emdev_t *dev = NULL, *dev1;
	int i;

	memcpy((void *)&info, (void *)(ih+1), 8);
	//校验客户端信息是否合法
	//...

	//init respone
	c->out->last += sizeof(impush_header_t);
	imdefault_header(ih, oh);

	//check ok, keep client info
	dev = hash_find_dev(p_imserver->hash_dev, info); //检查是否已经存在
	if(dev == NULL)
	{

		PT_Info("new client!\n");
		dev = hash_insert_dev(p_imserver->pool, p_imserver->hash_dev, info);
		if(dev == NULL)
		{
			PT_Warn("sign error...\n");
			imrespone_immediately(c->fd, ih->session_id, IMWARN_ERR);
			return NGX_ERROR; //此处应该立马构造一个回复包，报告错误
		}

		dev->devid = sequence_id++;
		dev_info_add(p_imserver, dev);
		dev->linfo = info;
	}
	else
		PT_Info("old client!\n");

	//push dev id to client

	oh->len = htons(4);
	*(uint32_t *)(oh + 1) =  htonl(dev->devid);
	c->out->last += 4;
	c->id = dev->devid;
	PT_Info("---sign, id:%u, info:%llu, address:%x\n", c->id, info, (int)c);

	return NGX_OK;
}

ngx_int_t impush_alive(impush_conn_t *c)
{
	//更新时间
	impush_header_t *ih = (impush_header_t *)(c->in->pos);
	impush_header_t *oh = (impush_header_t *)(c->out->pos);
	uint32_t	*d_id = (uint32_t *)(ih + 1);
	emdev_t *dev = NULL;

	//init respone
	c->out->last += sizeof(impush_header_t);
	imdefault_header(ih, oh);

	if(c->dev == NULL)
	{
		imrespone_immediately(c->fd, ih->session_id, IMWARN_ERR);
		return NGX_ERROR;
	}

	dev = c->dev;
	get_imtime(&dev->last_time);
	PT_Info("devid:%u alive, address:%x\n", dev->devid, dev->connect);
	//更新在该链接上的定时器事件

	return NGX_OK;	
}


//该函数用来处理，服务器推送消息后，客户端的返回信息，该消息不需要再次回写数据了

ngx_int_t impush_spush(impush_conn_t *c)
{
	return NGX_OK;	
}


ngx_int_t impush_cpush(impush_conn_t *c)
{
	impush_header_t *ih = (impush_header_t *)(c->in->pos);
	impush_header_t *oh = (impush_header_t *)(c->out->pos);
	emdev_t *dev = NULL;
	ngx_str_t msg;
	uint8_t buf[512];
	uint16_t len;
	uint64_t	info;

	//init respone
	c->out->last += sizeof(impush_header_t);
	imdefault_header(ih, oh);

	msg.data = (uint8_t *)(ih + 1);
	msg.len = ntohs(ih->len);

	if(c->dev == NULL)
	{
		printf("not found dev!\n");
		imrespone_immediately(c->fd, ih->session_id, IMWARN_ERR);
		return NGX_ERROR;
	}
	dev = c->dev;
	get_imtime(&dev->last_time);

	//put user to handle
	switch(ih->warn)
	{
		case 0:
			len = snprintf(buf, sizeof(buf), "recv:%-*.*s  respone client(%u): hello %u\n", msg.len, msg.len, msg.data, c->id, ntohs(ih->session_id));
			memcpy(c->out->last, buf, len);
			c->out->last += len;
			oh->len = htons(msg.len);
		break;
		case 1: //请求广播信息
			//权限认证：
			PT_Info("public broadcast....\n");

			im_add_msg(msg.data, msg.len);
		break;
		case 2: //请求推送信息给指定设备
			//权限认证：
			memcpy((void *)&info, (void *)(ih+1), 8);

			dev = hash_find_dev(p_imserver->hash_dev, info); //检查是否已经存在
			if(dev != NULL && dev->connect != NULL)
			{
				write(dev->connect->fd, msg.data+8, msg.len - 8);
			}
			break;
	}

	return NGX_OK;	
}

//系统默认注册的的钩子函数
//sign hook

static im_hook_t imsign_hookss =
{
	.hook = impush_sign,
	.priorty = 0,
};

static im_hook_t imlogin_hookss =
{
	.hook = impush_login,
	.priorty = 0,
};

static im_hook_t imlogout_hookss =
{
	.hook = impush_logout,
	.priorty = 0,
};

static im_hook_t imalive_hookss =
{
	.hook = impush_alive,
	.priorty = 0,
};

static im_hook_t imcpush_hookss =
{
	.hook = impush_cpush,
	.priorty = 0,
};

static im_hook_t imspush_hookss =
{
	.hook = impush_spush,
	.priorty = 0,
};


ngx_int_t init_imhook()
{
	ngx_int_t ret=0;

	init_env();

	//register your hook here! ...
	//register sign
	ret = hookregister(&imsign_hookss, IMPUSH_SIGN);

	//register login
	ret |= hookregister(&imlogin_hookss, IMPUSH_LOGIN);

	//register alive
	ret |= hookregister(&imalive_hookss, IMPUSH_ALIVE);

	//register logout
	ret |= hookregister(&imlogout_hookss, IMPUSH_LOGOUT);

	//register cpush
	ret |= hookregister(&imcpush_hookss, IMPUSH_CPUSH);

	//register spush
	ret |= hookregister(&imspush_hookss, IMPUSH_SPUSH);

	return ret;
}


