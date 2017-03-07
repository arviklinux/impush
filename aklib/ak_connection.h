/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#if 0
#ifndef _AK_H_INCLUDED_
#define _AK_H_INCLUDED_

#include "ak_core.h"

/*
typedef struct impush_conn_s impush_conn_t;

struct impush_conn_s
{
	ngx_socket_t fd;
	struct sockaddr sockaddr;
	socklen_t	socklen;
	
	ak_event_t	*read;
	ak_event_t	*write;
	ak_event_t	*close;
};
*/


typedef struct ak_listening_s  ak_listening_t;
typedef struct ak_connection_s ak_connection_t;

struct ak_listening_s
{
	ngx_socket_t fd;
	struct sockaddr	sockaddr;
	socklen_t	socklen;
	ngx_str_t	addr_text;

	int 	backlog;

    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;

	void	*servers; //array of ngx_http_in_addr_t, for example
	size_t	pool_size;
	size_t	post_accept_buffer_size;
	size_t	post_accept_timeout;

	ak_listening_t	*previous;

	unsigned	open:1;
	unsigned	remain:1;
	unsigned	ignore:1;
	unsigned	nonblocking:1;

};

typedef enum {
    NGX_ERROR_ALERT = 0,
    NGX_ERROR_ERR,
    NGX_ERROR_INFO,
    NGX_ERROR_IGNORE_ECONNRESET,
    NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
    NGX_TCP_NODELAY_UNSET = 0,
    NGX_TCP_NODELAY_SET,
    NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
    NGX_TCP_NOPUSH_UNSET = 0,
    NGX_TCP_NOPUSH_SET,
    NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


struct ak_connection_s
{
	void	*data;
	ngx_socket_t fd;
	ak_event_t	*read;
	ak_event_t	*write;
	ak_event_t	*close;

	struct sockaddr	*local_sockaddr;
	socklen_t	local_socklen;

	ngx_buf_t	*buffer;
	ngx_queue_t	queue;
	ak_listening_t *listening;

}
#endif




#endif
