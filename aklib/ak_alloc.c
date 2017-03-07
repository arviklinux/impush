/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include <malloc.h>

#include "ak_alloc.h"
#include "pt.h"

ngx_uint_t  ngx_pagesize = 4096;
//ngx_uint_t  ngx_pagesize_shift;
ngx_uint_t  ngx_cacheline_size = 32;

void *ngx_alloc(size_t size)
{
	void *p;

	p = malloc(size);
	if(p == NULL)
		PT_Error("malloc %u\n failed!\n", size);

	return p;
}


void *ngx_calloc(size_t size)
{
	void *p;
	p = malloc(size);
	if(p == NULL)
		PT_Error("malloc %u\n failed!\n", size);
	
	memset(p, 0, size);

	return p;
}

void *ngx_memalign(size_t alignment, size_t size)
{
    void  *p;
//函数memalign将分配一个由size指定大小，地址是alignment的倍数的内存块。参数alignment必须是2的幂！
    p = memalign(alignment, size); 
    if (p == NULL) {
		PT_Error("memalign %u\n failed!\n", size);
    }

    return p;
}

