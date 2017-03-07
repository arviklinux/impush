/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include "im_rbtimer.h"


ngx_rbtree_t	ak_rbtimer_rbtree;
static ngx_rbtree_node_t ak_rbtimer_sentinel;



ngx_int_t ak_event_timer_init()
{
	ngx_rbtree_init(&ak_rbtimer_rbtree, &ak_rbtimer_sentinel, ngx_rbtree_insert_timer_value);
	return NGX_OK;
}


ngx_msec_t ak_event_find_timer(ngx_msec_t current_time)
{
	ngx_msec_int_t	timer;
	ngx_rbtree_node_t *node, *root, *sentinel;

	if(ak_rbtimer_rbtree.root == &ak_rbtimer_sentinel)
		return NGX_TIMER_INFINITE;
	
	root = ak_rbtimer_rbtree.root;
	sentinel = ak_rbtimer_rbtree.sentinel;

	node = ngx_rbtree_min(root, sentinel);

	timer = (ngx_msec_int_t) (node->key - current_time);

	return (ngx_msec_t) (timer > 0 ? timer : 0);
}

void ak_event_expire_timers(ngx_msec_t current_time)
{
	ak_event_t*ev;
	ngx_rbtree_node_t *node, *root, *sentinel;

	sentinel = ak_rbtimer_rbtree.sentinel;

	for( ;; )
	{
		root = ak_rbtimer_rbtree.root;
		if(root == sentinel)
			return;
		
		node = ngx_rbtree_min(root, sentinel);
		/* node->key > ngx_current_time */
		if( (ngx_msec_int_t)(node->key - current_time) > 0 )
			return;
		
		ev = rbtree_containerof(node, ak_event_t, timer);
		ngx_rbtree_delete(&ak_rbtimer_rbtree, &ev->timer);

		ev->timer_set = 0;
		ev->timedout = 1;
		ev->handler(ev);
	}
}

void ak_event_cancel_timers(void)
{
	ak_event_t*ev;
	ngx_rbtree_node_t *node, *root, *sentinel;

	sentinel = ak_rbtimer_rbtree.sentinel;

	for( ;; )
	{
		root = ak_rbtimer_rbtree.root;
		if(root == sentinel)
			return;
		
		node = ngx_rbtree_min(root, sentinel);

		ev = rbtree_containerof(node, ak_event_t, timer);
		if(!ev->cancelable)
			return;
		
		ngx_rbtree_delete(&ak_rbtimer_rbtree, &ev->timer);
		ev->timer_set = 0;
		ev->handler(ev);
	}

}

void ak_event_del_timer(ak_event_t *ev)
{
	ngx_rbtree_delete(&ak_rbtimer_rbtree, &ev->timer);
	ev->timer_set = 0;
}

void ak_event_add_timer(ak_event_t *ev, ngx_msec_t timer, ngx_msec_t current_time)
{
	ngx_msec_t      key;
	ngx_msec_int_t  diff;

	key = current_time + timer;
	if (ev->timer_set)
	{
		diff = (ngx_msec_int_t) (key - ev->timer.key);
		if (ngx_abs(diff) < NGX_TIMER_LAZY_DELAY)
			return;
		
		ak_del_timer(ev);
	}

	ev->timer.key = key;

	ngx_rbtree_insert(&ak_rbtimer_rbtree, &ev->timer);
	ev->timer_set = 1;
}


