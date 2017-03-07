/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include "im_ss.h"

//extern server_status_t *p_imserver;

#define dev_hash_key(key)	((uint64_t)key)

static struct timeval imtv;

void update_imtime()
{
	gettimeofday(&imtv, NULL);
}

void get_imtime(struct timeval *imtime)
{
	*imtime = imtv;
}

long delta_imtime_ms(struct timeval *old_time)
{
	long delta_ms=0;

	delta_ms = (imtv.tv_sec - old_time->tv_sec)*60 + (imtv.tv_usec - old_time->tv_usec)/1000;
	return delta_ms;
}

long delta_imtime_s(struct timeval *old_time)
{
	return imtv.tv_sec - old_time->tv_sec;
}

//设备信息哈希操作方法
emdev_t *create_dev_node(ngx_pool_t *p)
{
	//emdev_t *dev1;
	emdev_t *d;
	d = (emdev_t *)ngx_pcalloc(p, sizeof(emdev_t));
	if(d == NULL)
		return d;

	//dev1 = rbtree_find_dev(p_imserver, 106);
	//	if(dev1 != NULL)
	//	printf("----000--see id 106 , id:%u, addr:%x\n", dev1->devid, dev1->connect);

	INIT_HLIST_NODE(&d->h_devnode);
	return d;
}

ngx_int_t init_dev_hash(ngx_pool_t *p, struct hlist_head **dev_table)
{
	int i;
	struct hlist_head *table = NULL;

	table = (struct hlist_head *)ngx_pcalloc( p, sizeof(struct hlist_head) * DEV_HASH_SIZE );
	if(table == NULL)
		return NGX_ERROR;
	
	for(i = 0; i<DEV_HASH_SIZE; i++)
		INIT_HLIST_HEAD(&table[i]);
	
	*dev_table = table;
	return NGX_OK;
}


emdev_t *hash_insert_dev(ngx_pool_t *p, struct hlist_head *dev_table, uint64_t key)
{
	struct hlist_head *phead = NULL;
	emdev_t *dev_n = NULL, *dev1;

	phead = &dev_table[dev_hash_key(key) % DEV_HASH_SIZE];
	hlist_for_each_entry(dev_n, phead, h_devnode)
	{
		if( dev_n->linfo == key )
		{
			return dev_n;
		}
	}

	dev_n = create_dev_node(p);
	if(dev_n == NULL)
		return NULL;
	
	dev_n->linfo = key;
	hlist_add_head(&dev_n->h_devnode, phead);

	return dev_n;
}

emdev_t *hash_find_dev(struct hlist_head *dev_table, uint64_t key)
{
	struct hlist_head *phead = NULL;
	emdev_t *dev_n = NULL;

	phead = &dev_table[dev_hash_key(key) % DEV_HASH_SIZE];
	hlist_for_each_entry(dev_n, phead, h_devnode)
	{
		if( dev_n->linfo == key )
			return dev_n;
	}
	
	return NULL;
}

emdev_t *rbtree_find_dev(server_status_t *s, ngx_rbtree_key_t dev_id)
{
	ngx_rbtree_node_t *tmpnode = s->rb_dev.root;
	emdev_t *dev;

	while(tmpnode != &s->rb_sentinel)
	{
		if(dev_id != tmpnode->key)
		{
			tmpnode = (dev_id < tmpnode->key)?  tmpnode->left : tmpnode->right;
			continue;
		}

		dev = rbtree_containerof(tmpnode, emdev_t, r_devnode);
		return dev;
	}

	return NULL;
}


//if this fun failed, you must close the application
ngx_int_t init_server_status(server_status_t *s)
{
	ngx_int_t ret;

	ngx_memset(s, 0, sizeof(server_status_t));

	s->pool = ngx_create_pool(NGX_DEFAULT_POOL_SIZE);
	if(s->pool == NULL)
		return NGX_ERROR;

	//init dev rbtree
	ngx_rbtree_init(&s->rb_dev, &s->rb_sentinel, ngx_rbtree_insert_value);

	//init hash
	if(init_dev_hash(s->pool, &s->hash_dev) != NGX_OK)
	{
		ret = NGX_ERROR;
		goto exit1;
	}

	return NGX_OK;

exit1:

	ngx_destroy_pool(s->pool);
	return ret;
}


void dev_info_add(server_status_t *s, emdev_t *dev)
{
	//添加到设备红黑树列表
	dev->r_devnode.key = dev->devid;
	dev->status = 0;//离线
	ngx_rbtree_insert(&s->rb_dev, &dev->r_devnode);
	return;
}

ngx_int_t dev_online(server_status_t *s, ngx_rbtree_key_t dev_id)
{
	emdev_t *dev = NULL;
	dev = rbtree_find_dev(s, dev_id);
	if(dev == NULL)
		return NGX_ERROR;
	
	dev->status = 1;
	return NGX_OK;
}

ngx_int_t dev_offline(server_status_t *s, ngx_rbtree_key_t dev_id)
{
	emdev_t *dev = NULL;
	dev = rbtree_find_dev(s, dev_id);
	if(dev == NULL)
		return NGX_ERROR;
	
	dev->status = 1;
	return NGX_OK;
}



/*
ngx_int_t new_dev_notice(server_status_t *s, emdev_t *dev)
{
	//添加到红黑树
	ngx_rbtree_insert(&s->offline_rb, &dev->node);

	//映射hash表

	return NGX_OK;
}

*/


