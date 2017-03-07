/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

/*
#include "im_hook.h"

//队列按照优先级从高到低排列（值越小优先级越高）
static ngx_queue_t hooksign_l, hooklogin_l, hookalive_l, hookcpush_l, hooklogout_l;


ngx_int_t init_env()
{
	ngx_queue_init(&hooksign_l);
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

		default:
			ret = NGX_ERROR;
	}

	return ret;
}


ngx_int_t runhook_queue(im_hook_t *h, impush_conn_t *c)
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
	}

	return ret;
}

*/

/*
//系统默认注册的的钩子函数
//sign hook

static im_hook_t imsign_hookss
{
	.hook = impush_sign;
	.priorty = 0;
}

static im_hook_t imlogin_hookss
{
	.hook = impush_login;
	.priorty = 0;
}

static im_hook_t imlogout_hookss
{
	.hook = impush_logout;
	.priorty = 0;
}

static im_hook_t imalive_hookss
{
	.hook = impush_alive;
	.priorty = 0;
}

static im_hook_t imcpush_hookss
{
	.hook = impush_cpush;
	.priorty = 0;
}


ngx_int_t init_imhook()
{
	ngx_int_t ret;

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

	return ret;
}

*/