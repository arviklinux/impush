/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#ifndef  _IM_EPOLL_H_INCLUDED_
#define _IM_EPOLL_H_INCLUDED_

#include "ak.h"
#include "im_prase.h"
#include "im_rbtimer.h"

#define MAX_EPOLL_EVENTS	1000
#define MAX_EPOLL_CONNECTION	1024
#define IM_MSG_SIZE_LIMIT	2000

typedef struct im_epoll_s	im_epoll_t;

struct im_epoll_s
{
	ngx_socket_t epfd;
	
	struct epoll_event epoll_evs[MAX_EPOLL_EVENTS];
};

ngx_int_t im_epoll_init();
ngx_int_t ak_epoll_done();
ngx_int_t im_epoll_add_connection(impush_conn_t *c);
ngx_int_t im_epoll_del_connection(impush_conn_t *c);
ngx_int_t im_epoll_process_events(ngx_msec_t timeout);


int imconn_accept(impush_conn_t *c);
ngx_int_t impush_read_data(impush_conn_t *c);
ngx_int_t impush_handle_data(impush_conn_t *c);
int imconn_close(impush_conn_t *c);

#endif

