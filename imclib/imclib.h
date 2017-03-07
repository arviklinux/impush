/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#ifndef __IMCLIB_H
#define __IMCLIB_H

#include <stdlib.h> //alloc
#include <string.h> //memset  memcpy
#include <stdint.h> //size_t
#include <sys/types.h> //u_char
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>


#define CURRENT_VER 1

//im msg type
#define	IMPUSH_SERACK	0
#define IMPUSH_CLIACK	1
#define IMPUSH_SIGN		3
#define IMPUSH_ACTIVATE	4
#define IMPUSH_LOGIN	5
#define IMPUSH_LOGOUT	7
#define IMPUSH_ALIVE	9
#define IMPUSH_SPUSH	10
#define IMPUSH_CPUSH	11

typedef struct impush_header_s	impush_header_t;
typedef struct imclib_s imclib_t;
typedef struct immsglist_s immsglist_t;
struct impush_cb_s impush_cb_t;

typedef int (*imcb)(int8_t *msg, uint16_t msglen);
typedef int (*f_imloop)(struct imclib_s *imc);

struct impush_header_s
{
	uint8_t	ver;
	uint8_t type;
	uint8_t warn;
	uint8_t reserve;
	uint16_t len;
	uint16_t session_id;
};

struct immsglist_s
{
	immsglist_t *next;
	int8_t *msg;
	int16_t len;
};

struct impush_cb_s
{
	uint8_t type;
	uint8_t warn;
	imcb cb;
};

struct imclib_s
{
	int8_t info[8]; //设备信息
	int imfd; //
	int run; //消息循环标志
	uint32_t id; //设备id, 本地字节序
	uint32_t heartbeat; //心跳时间
	uint16_t port; //服务器端口号
	int16_t maxlen; //消息最长大小
	uint32_t rtt; //读写套接字超时时间
	immsglist_t *msglist; //消息推送链表
	imcb cb; //消息回调函数
	f_imloop imloop;
	char sipstr[24];
	uint32_t n_sip;
	int imloopcount;
	int imloopcontinue;
};


void init_imc(imclib_t *imc);
int im_aliveloop(imclib_t *imc);
int register_spush_cb(imcb cb, int force, imclib_t *imc);
int im_addmsg(int8_t *buf, int16_t len, int8_t method, imclib_t *imc);

#endif