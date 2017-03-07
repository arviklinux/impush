/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#pragma once

#ifndef AK_ALLOC_H
#define AK_ALLOC_H

#include "ak_config.h"

extern ngx_uint_t  ngx_pagesize;
extern ngx_uint_t  ngx_cacheline_size;

//#define ngx_memzero(buf, n)	(void)memset(buf, 0, n)  //define in ak_string.h
#define ngx_free		free

void *ngx_alloc(size_t size);
void *ngx_calloc(size_t size);
void *ngx_memalign(size_t alignment, size_t size);

#endif
