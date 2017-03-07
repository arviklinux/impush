/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/
#pragma once

#ifndef __AK_CONFIG_H
#define __AK_CONDIF_H

#include <stdlib.h> //alloc
#include <string.h> //memset  memcpy
#include <stdint.h> //size_t
#include <sys/types.h> //u_char
#include <stdarg.h> //va_list
#include <stddef.h> //offsetof
#include <sys/socket.h> //socket
#include <sys/epoll.h> //epoll
#include <netinet/in.h> //inet
#include <netinet/tcp.h> //flag TCP_CORK TCP_NODELAY ...
#include <arpa/inet.h>
#include <fcntl.h> //fcnt
#include <unistd.h>
#include <errno.h>

#include "../config.in"

#ifdef MACHINE_32BIT
//////////////32位机器//////////////////////
//预定义控制区
//如果目标机器是32位机器，则定义该宏，否则不定义
#define NGX_PTR_SIZE  4

//以下分别表示32位机器中size_t,off_t,time_t类型最大值 
#ifndef NGX_MAX_SIZE_T_VALUE
#define NGX_MAX_SIZE_T_VALUE	0x7fffffffL
#endif

#ifndef NGX_MAX_OFF_T_VALUE
#define NGX_MAX_OFF_T_VALUE		0x7fffffffL
#endif

#ifndef NGX_MAX_TIME_T_VALUE
#define NGX_MAX_TIME_T_VALUE		0x7fffffffL
#endif

typedef int32_t                     ngx_atomic_int_t;
typedef uint32_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)
///////////////////////////////////////
#else  //64_bit_machine
#define NGX_PTR_SIZE  8

//以下分别表示32位机器中size_t,off_t,time_t类型最大值 
#ifndef NGX_MAX_SIZE_T_VALUE
#define NGX_MAX_SIZE_T_VALUE	9223372036854775807LL
#endif

#ifndef NGX_MAX_OFF_T_VALUE
#define NGX_MAX_OFF_T_VALUE		9223372036854775807LL
#endif

#ifndef NGX_MAX_TIME_T_VALUE
#define NGX_MAX_TIME_T_VALUE		9223372036854775807LL
#endif

typedef int64_t                     ngx_atomic_int_t;
typedef uint64_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#endif



#define ngx_cdecl
#define ngx_libc_cdecl

typedef intptr_t        ngx_int_t;
typedef uintptr_t       ngx_uint_t;
typedef intptr_t        ngx_flag_t;

#define NGX_INT32_LEN   (sizeof("-2147483648") - 1)
#define NGX_INT64_LEN   (sizeof("-9223372036854775808") - 1)

#if (NGX_PTR_SIZE == 4)
#define NGX_INT_T_LEN   NGX_INT32_LEN
#define NGX_MAX_INT_T_VALUE  2147483647

#else
#define NGX_INT_T_LEN   NGX_INT64_LEN
#define NGX_MAX_INT_T_VALUE  9223372036854775807
#endif

#ifndef NGX_ALIGNMENT
#define NGX_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#endif


#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


//异常终止一个进程
#define ngx_abort       abort

/* TODO: platform specific: array[NGX_INVALID_ARRAY_INDEX] must cause SIGSEGV */
#define NGX_INVALID_ARRAY_INDEX 0x80000000


/* TODO: auto_conf: ngx_inline   inline __inline __inline__ */
#ifndef ngx_inline
#define ngx_inline      inline
#endif

#ifndef INADDR_NONE  /* Solaris */
#define INADDR_NONE  ((unsigned int) -1)
#endif

#ifdef MAXHOSTNAMELEN
#define NGX_MAXHOSTNAMELEN  MAXHOSTNAMELEN
#else
#define NGX_MAXHOSTNAMELEN  256
#endif


#define NGX_MAX_UINT32_VALUE  (uint32_t) 0xffffffff
#define NGX_MAX_INT32_VALUE   (uint32_t) 0x7fffffff




#endif //ak_config.h