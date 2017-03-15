/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#ifndef _PUSH_PRASE_H_INCLUDED_
#define _PUSH_PRASE_H_INCLUDED_

#include "ak.h"

#define CURRENT_VER	1
#define IMPUSH_ALIVE_TIMEOUT   60

//im msg type
//#define	IMPUSH_SERACK	0
//#define IMPUSH_CLIACK	1
#define IMPUSH_SIGN		3
//#define IMPUSH_ACTIVATE	4
#define IMPUSH_LOGIN	5
#define IMPUSH_LOGOUT	7
#define IMPUSH_ALIVE	9
#define IMPUSH_SPUSH	10
#define IMPUSH_CPUSH	11

//im warn
#define IMWARN_OK			0
#define IMWARN_ERR			1
#define IMWARN_BUSY			2
#define IMWARN_TOOLONG		3
#define IMWARN_DECLINED		4
#define IMWARN_NOSIGN		5



//im chat status
#define IMCHAT_OPEN			0x0001
#define IMCHAT_READING		0x0002
#define IMCHAT_HANDLING		0x0004
#define IMCHAT_WRITING		0x0008
#define IMCHAT_KEEPING		0x0010
#define IMCHAT_WAITCLOSE	0x0020


typedef struct impush_header_s	impush_header_t;
//typedef struct impush_s	impush_t;
typedef struct impush_conn_s impush_conn_t;
typedef struct emdev_s emdev_t;


//typedef  int (*handle_im)(impush_t *s);
typedef  ngx_int_t (*handle_conn)(impush_conn_t *c);

struct impush_header_s
{
	uint8_t	ver;
	uint8_t type;
	uint8_t warn;
	uint8_t reserve;
	uint16_t len;
	uint16_t session_id;
};

struct emdev_s;

struct impush_conn_s
{
	ngx_socket_t fd;
	struct sockaddr_in sockaddr;
	socklen_t	socklen;
	struct emdev_s *dev;
	uint32_t id;
	time_t lasttime;

	unsigned fdtype:1; // 1:代表监听套接字，0：代表accept生成的套接字
	unsigned chatting:1; //决定是否重置内存池  1:代表正在chatting，在chat_pool中创建了in和out, 此时不能重置chat_pool， 0：需要重置chat_pool
	unsigned dev_associated:1;//该链接是否绑定了具体的设备
	unsigned set_timer:1;

	handle_conn accept_handle;

	handle_conn	imread;
	handle_conn	imhandle;
	handle_conn	imwrite;
	handle_conn imclose;


	uint32_t chatstatus; //交谈状态 start, reading, handling, writing, keeping, close
	ngx_pool_t	*chat_pool; //不为空代表已经创建了内存池

	ngx_buf_t *in;  //接收的报文存放到这里
	ngx_buf_t *out; //组建发出的报文存放到这里
	//impush_t	*im;
};

struct emdev_s
{
	ngx_rbtree_node_t r_devnode; //设备红黑树节点，以id为关键字
	struct hlist_node h_devnode; //设备信息节点，以客户端info为关键字

	uint32_t	devid;
	uint32_t	status; // 1:在线 0：离线

	union
	{
		uint64_t linfo;
		uint8_t ainfo[8];
	};

	impush_conn_t *connect;
	time_t last_logintime;
	struct timeval last_time;
};

typedef struct im_hook_s im_hook_t;

struct im_hook_s
{
	ngx_queue_t que;
	handle_conn hook;
	//int hooktype; //hook消息节点类型， sign, login ...
	int priorty; //优先级
};

typedef struct impush_msg_s impush_msg_t;

struct impush_msg_s
{
	impush_msg_t *next;
	uint16_t len; //消息长度 + 报头长度
	uint16_t id; //发送消息的id,用以确认不同的消息
	uint8_t *buf; //消息缓冲区，包括报头8个字节区域
};


ngx_int_t im_add_msg(int8_t *buf, int16_t len);
ngx_int_t impush_spush_pump();
ngx_int_t impush_timeout_clear();

ngx_int_t hookregister(im_hook_t *r, int type);
ngx_int_t runhook_queue(ngx_queue_t *head, impush_conn_t *c);

int imrespone_immediately(int fd, int16_t n_id, uint8_t warn_type);
ngx_int_t init_imhook();


#endif
