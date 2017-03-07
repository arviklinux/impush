/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#if 0

#ifndef  _AK_EPOLL_H_INCLUDED_
#define _AK_EPOLL_H_INCLUDED_

#include "ak_core.h"
#include "ak_socket.h"

#define MAX_EPOLL_EVENTS	100
#define MAX_EPOLL_CONNECTION	1024

typedef struct ak_epoll_module_s	ak_epoll_module_t;

struct ak_epoll_module_s
{
	ngx_socket_t epfd;
	
	struct epoll_event epoll_evs[100];
};

#endif


#endif