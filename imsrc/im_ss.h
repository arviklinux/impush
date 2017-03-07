/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#ifndef _IM_SS_H_INCLUDED_
#define _IM_SS_H_INCLUDED_

#include "ak.h"
#include "im_prase.h"

//关键字元素的最大长度
#define DEV_INFO_LEN	8

//hash表长
#define DEV_HASH_SIZE	10007

//extern server_status_t *p_imserver;
typedef struct server_status_s server_status_t;



struct server_status_s
{
	struct hlist_head *hash_dev;
	ngx_rbtree_t rb_dev;
	ngx_rbtree_node_t rb_sentinel;
	time_t lasttimecheck;

	ngx_pool_t  *pool; //用于存储设备信息，永不释放(可由数据库替代)，
};

void update_imtime();
void get_imtime(struct timeval *imtime);
long delta_imtime_ms(struct timeval *old_time);
long delta_imtime_s(struct timeval *old_time);


//emdev_t *create_dev_node(ngx_pool_t *p);
//ngx_int_t init_dev_hash(ngx_pool_t *p, struct hlist_head **dev_table);
emdev_t *hash_insert_dev(ngx_pool_t *p, struct hlist_head *dev_table, uint64_t key);
emdev_t *hash_find_dev(struct hlist_head *dev_table, uint64_t key);
emdev_t *rbtree_find_dev(server_status_t *s, ngx_rbtree_key_t dev_id);

ngx_int_t init_server_status(server_status_t *s);
void dev_info_add(server_status_t *s, emdev_t *dev);
ngx_int_t dev_online(server_status_t *s, ngx_rbtree_key_t dev_id);
ngx_int_t dev_offline(server_status_t *s, ngx_rbtree_key_t dev_id);



#endif

