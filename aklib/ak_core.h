/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#pragma once

#ifndef _AK_CORE_H
#define _AK_CORE_H

#include "ak_config.h"

typedef struct ngx_pool_s        ngx_pool_t;
typedef struct ngx_array_s		ngx_array_t;

#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6

#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"

#define ngx_abs(value)       (((value) >= 0) ? (value) : - (value))
#define ngx_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define ngx_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))


#include "ak_alloc.h"
#include "ak_buf.h"
#include "ak_palloc.h"
#include "ak_string.h"
#include "ak_array.h"
#include "ak_queue.h"
#include "ak_list.h"
#include "ak_rbtree.h"
#include "ak_radix_tree.h"
#include "ak_md5.h"
#include "ak_socket.h"

#endif