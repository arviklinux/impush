/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#if 0
#include "ak_epoll_module.h"


ngx_int_t ak_epoll_init(ak_epoll_module_t *epoll_m)
{
	ngx_memzero(epoll_m, siezof(ak_epoll_module_t));

	epoll_m->epfd = create(MAX_EPOLL_CONNECTION);
	if(epoll_m->epfd == -1)
		return NGX_ERROR;

	return NGX_OK;
}

ngx_int_t ak_epoll_done(ak_epoll_module_t *epoll_m)
{
	close(epoll_m->epfd);
	return NGX_OK;
}


ngx_int_t ak_epoll_add_event(ak_epoll_module_t *epoll_m, ngx_event_t *ev)
{
	return NGX_OK;
}

ngx_int_t ak_epoll_add_connection(ak_epoll_module_t *epoll_m, ak_connection_t *c)
{
	struct epoll_event ee;

	ee.events = EPOLLIN|EPOLLET|EPOLLRDHUP; //不关注写事件
	ee.data.ptr = (void *) c;

	if(epoll_ctl(epoll_m->epfd, EPOLL_CTL_ADD, c->fd, &ee) == -1)
		return NGX_ERROR;

	c->read->active = 1;
	c->write->active = 1;

	return NGX_OK;
}

ngx_int_t ngx_epoll_del_connection(ngx_connection_t *c, ngx_uint_t flags)
{
	struct epoll_event  ee;

	ee.events = 0;
	ee.data.ptr = NULL;
	if (epoll_ctl(epoll_m->epfd, EPOLL_CTL_DEL, c->fd, &ee) == -1)
		return NGX_ERROR;
	
	c->read->active = 0;
	c->write->active = 0;

	return NGX_OK;
}



//该函数会阻塞超时
ngx_int_t ak_epoll_process_events(ak_epoll_module_t *epoll_m, ngx_msec_t timeout)
{
	int events;
	uint32_t revents;
	int i;
	ak_event_t *rev;


	events = epoll_wait(epoll_m->epfd, epoll_m->epoll_evs, timeout);
	if(events == -1)
		return NGX_ERROR;
	
	if(events == 0) //timeout
		return NGX_OK;
	
	for (i = 0; i < events; i++)
	{
		c = (ngx_connection_t *) (epoll_m->epoll_evs[i].data.prt);
		rev = c->read;

		if(c->fd == -1)
			continue;
		
		revents = epoll_m->epoll_evs[i].events;
		if(revents & EPOLLHUP )
		{
			rev->handler(rev);
		}


	}

	return NGX_OK;
}

#endif
