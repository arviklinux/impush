/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#ifndef _NGX_RBTIMER_H_INCLUDED_
#define _NGX_RBTIMER_H_INCLUDED_

#include "ak_core.h"
#include "im_prase.h"

#define NGX_TIMER_LAZY_DELAY	300

typedef ngx_rbtree_key_t	ngx_msec_t;
typedef ngx_rbtree_key_int_t	ngx_msec_int_t;

#define NGX_TIMER_INFINITE	(ngx_msec_t)-1

typedef struct ak_event_s ak_event_t;
typedef void (*ak_event_handler_pt)(ak_event_t *ev);

#define ak_del_timer	ak_event_del_timer
#define ngx_event_t		ak_event_t

struct ak_event_s
{
	ngx_rbtree_node_t timer;

	//到期时间
	time_t time; //单位ms，32位，大约49天溢出

	//回调函数
	ak_event_handler_pt handler;

	impush_conn_t *c;

	void *data;

	uint32_t	epoll_event_type; //EPOLLIN|EPOLLOUT|EPOLLET|EPOLLRDHUP EPOLLRDHUP代表对端断开连接，对方的close会同时触发EPOLLRDHUP和EPOLLIN
	
	unsigned	active:1; //表示是否已经添加过epoll处理
	unsigned	disabled:1;
	unsigned	timer_set:1;
	unsigned	timedout:1;
	unsigned	closed:1;
	unsigned	cancelable:1;

	ngx_queue_t		queue;
};




ngx_int_t ak_event_timer_init();
ngx_msec_t ak_event_find_timer(ngx_msec_t current_time);
void ak_event_expire_timers(ngx_msec_t current_time);
void ak_event_cancel_timers(void);

extern ngx_rbtree_t ak_rbtimer_rbtree;

void ak_event_del_timer(ak_event_t *ev);
void ak_event_add_timer(ak_event_t *ev, ngx_msec_t timer, ngx_msec_t current_time);


#endif
