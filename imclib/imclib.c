/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include "imclib.h"

//static imclib_t imc;
//static immsglist_t *pmsghead = NULL;
static int16_t s_id=0;

//static imcb imspushcb = NULL;


/*
根据网卡名称获取其mac地址，缓冲区mac必须大于6个字节
*/
int GetLocalDevMac0X(char *devname, uint8_t *mac)
{
	struct ifreq ifr;
	int fd;

	memset(&ifr, 0, sizeof(ifr));
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1)
		return -1;
	
	strcpy(ifr.ifr_name, devname);
	if(ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
	{
		close(fd);
		return -2;
	}

	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
	close(fd);

	return 0;
}

void setnonblocking(int sock)
{
        int opts;
        opts = fcntl(sock, F_GETFL);

        if(opts < 0) {
                perror("fcntl(sock, GETFL)");
                exit(1);
        }

        opts = opts | O_NONBLOCK;

        if(fcntl(sock, F_SETFL, opts) < 0) {
                perror("fcntl(sock, SETFL, opts)");
                exit(1);
        }
}

int nob_write(int fd, char *buf, size_t len, uint32_t timeout)
{
	int ret=-1;
	int nsend = 0;

	while(nsend < len)
	{
		ret = write(fd, buf, len - nsend);
		if(ret > 0)
		{
			nsend += ret;
		}
		if(ret == 0) //套接字关闭
		{
			break;
		}
		else
		{
			if(EINTR == errno)//连接正常，操作被中断，可继续发送
				continue;
			else if(EWOULDBLOCK == errno || EAGAIN== errno)//连接正常，但发送缓冲区没有空间，等待下一次发
			{
				if(timeout-- < 0)
					sleep(1);
					continue;
			}
			else  //出错
				break;
		}
	}

	return ret;
}

int nob_read(int fd, char *buf, size_t len, uint32_t timeout)
{
	int ret = -1;

	do
	{
		ret = read(fd, buf, len);
		if(ret >= 0) //ret=0表示套接字关闭，读到fin
			return ret;
		else if(ret < 0)
		{
			if(EINTR == errno || EWOULDBLOCK == errno || EAGAIN== errno) 
			{
				ret = -2; //超时
				sleep(1); 
			}
			else 
				ret = -1;
		}
	}while(timeout--);

	return ret;
}


static inline void c_imdefault_header(impush_header_t *oh)
{
	oh->ver = CURRENT_VER;
	oh->type = 0; 
	oh->warn = 0; 
	oh->reserve = 0; 
	oh->len = htons(0); 
	oh->session_id = htons(s_id++);
}


/*
注册信息：
fd: 非阻塞套接字
info：impush协议注册用的信息
n_getid: 服务器返回的id存放位置
rtt: 读写数据超时时间
*/
int c_imsign(int fd, uint8_t *info, uint32_t *n_getid, uint32_t rtt)
{
	uint8_t buf[64];
	impush_header_t *im = (impush_header_t *)buf;
	uint8_t *p = &buf[8];
	int ret=0;

	c_imdefault_header(im);
	im->type = 3;
	im->len = htons(8);
	memcpy(p, info, 8);
	//write(fd, buf, 16);
	ret = nob_write(fd, buf, 16, rtt);
	if(ret <= 0)
		return -1;

	memset(buf, 0, sizeof(buf));
	ret = nob_read(fd, buf, 12, rtt); //12字节
	if(ret <= 0)
		return -2;

	if(ntohs(im->len) != 4 )
		return -3; //解析版本不对，1.0版本消息内容必须为4字节uint32_t的id

	if(im->warn == 0)
	{
		*n_getid = ntohl(*(uint32_t *)p);
		printf("sign success! get id:%u\n", *n_getid);
		ret = 0;
	}
	else
	{
		ret = -1;
		printf("sign error, warn:%d\n", im->warn);
	}

	return ret;
}

/*
登录服务器
fd: 非阻塞套接字
info：impush协议注册用的信息
n_getid: 服务器返回的id存放位置
rtt: 读写数据超时时间
*/

int c_imlogin(int fd, uint32_t n_id, uint32_t rtt)
{
	uint8_t buf[24];
	impush_header_t *im = (impush_header_t *)buf;
	uint8_t *p = &buf[8];
	int ret=0;

	c_imdefault_header(im);
	im->type = 5;
	im->len = htons(4);
	
	*(uint32_t *)p = htonl(n_id);

	ret = nob_write(fd, buf, 12, rtt);
	if(ret <= 0)
		return -1;

	memset(buf, 0, sizeof(buf));
	ret = nob_read(fd, buf, 8, rtt); //12字节
	if(ret <= 0)
		return -2;

	if(ntohs(im->len) != 0)
		return -3;
	
	if(im->warn == 0)
	{
		printf("login success!\n");
		ret = 0;
	}
	else
	{
		ret = -1;
		printf("login error, warn:%d\n", im->warn);
	}

	return ret;
}

int c_imalive(int fd, uint32_t rtt)
{
	uint8_t buf[64];
	impush_header_t *im = (impush_header_t *)buf;
	uint8_t *p = &buf[8];
	int ret=0;

	printf("client alive...\n");
	c_imdefault_header(im);
	im->type = 9;

	//write(fd, buf, 8);
	ret = nob_write(fd, buf, 8, rtt);
	if(ret <= 0)
		return -1;

	memset(buf, 0, sizeof(buf));
	//read(fd, buf, sizeof(buf)); //8个
	ret = nob_read(fd, buf, 8, rtt); //12字节
	if(ret <= 0)
		return -2;

	if(ntohs(im->len) != 0)
		return -3;

	if(im->warn == 0)
	{
		printf("alive success!\n");
		ret = 0;
	}
	else
	{
		ret = -4;
		printf("alive error, warn:%d\n", im->warn);
	}

	return ret;
}



//int im_connect(int8_t *ipstr, uint16_t port, int8_t *info, imclib_t *imc)
int im_connect(imclib_t *imc)
{
	int cfd;
	struct sockaddr_in serveraddr;
	uint32_t rtt = imc->rtt;

	//memcpy(imc->info, info, 8);
	cfd = socket(AF_INET, SOCK_STREAM, 0); //异步socket
	if(cfd == -1)
		return -1;

	imc->imfd = cfd;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = imc->n_sip; //inet_addr(ipstr);//htonl(INADDR_ANY); //inet_addr("42.96.130.249");
	serveraddr.sin_port = htons(imc->port);

	if(connect(cfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr) ) < 0)
	{
		printf("connect server failed, errno:%d\n", errno);
		return -2;
	}

	setnonblocking(cfd);
	if(c_imsign(cfd, imc->info, &(imc->id), rtt) != 0)
		return -3;
	
	if(c_imlogin(cfd, imc->id, rtt) != 0 )
		return -4;

	if(c_imalive(cfd, rtt) != 0)
		return -5;

	return 0;
}

int im_addmsg(int8_t *buf, int16_t len, int8_t method, imclib_t *imc)
{
	immsglist_t *p = imc->msglist, *p1;
	int8_t *b;
	impush_header_t *im;
	static uint16_t s_id;

	
	if(len > imc->maxlen)
		return -1;

	p = (immsglist_t *)malloc(sizeof(immsglist_t));
	if(p == NULL)
		return -1;
	
	b = malloc(len + sizeof(impush_header_t) );
	if(b == NULL)
	{
		free(p);
		return -2;
	}

	im = (impush_header_t *)b;
	memcpy(b + sizeof(impush_header_t), buf, len);
	im->ver = CURRENT_VER;
	im->type = 11; 
	im->warn = method; 
	im->reserve = 0; 
	im->len = htons(len); 
	im->session_id = htons(s_id++);
	p->msg = b;
	p->len = len + sizeof(impush_header_t);
	p->next = NULL;

	if(imc->msglist == NULL)
	{
		imc->msglist = p;
		return 0;
	}
	p1 = imc->msglist;
	while(p1->next != NULL)
		p1 = p1->next;

	p1->next = p;
	
	return 0;
}

int register_spush_cb(imcb cb, int force, imclib_t *imc)
{
	if(cb == NULL)
		return -1;

	if(force == 1)
	{
		imc->cb = cb;
		return 0;
	}

	if(imc->cb == NULL)
	{
		imc->cb = cb;
		return 0;
	}
	
	return -1;
}


int im_readcall(imclib_t *imc)
{
	static int8_t buf[2000];
	impush_header_t *im = (impush_header_t *)buf;
	int8_t *msg = buf + 8;
	int ret;
	int16_t msglen;
	imcb imspushcb = imc->cb;

	memset(buf, 0, 8);
	ret = nob_read(imc->imfd, buf, 8, imc->rtt);
	if(ret <= 0)
		return ret;

	if(im->ver != CURRENT_VER)
		return -2;

	msglen = ntohs(im->len);
	if(msglen != 0)
	{
		if(nob_read(imc->imfd, msg, msglen, 1 ) <= 0)
			return ret;
	}

	switch(im->type)
	{
		case IMPUSH_SPUSH:
			if(imspushcb != NULL)
			{
				imspushcb(msg, msglen);
			}

		break;

		default:
			break;
	}

	return 1;
}


int im_aliveloop(imclib_t *imc)
{
	int ret = 0;
	immsglist_t *p = imc->msglist, *p1;
	time_t lasttime, currenttime;
	uint8_t buf[1024];
	impush_header_t *im = (impush_header_t *)buf;
	int imloopcont=0;

	c_imdefault_header(im);
	im->type = 9;

	lasttime = currenttime = time(NULL);
	do
	{
		//周期执行用户自定义函数
		if(imc->imloop != NULL && imloopcont++ >= imc->imloopcount)
		{
			imloopcont = 0;
			if(imc->imloopcontinue>0)
			{
				imc->imloopcontinue--;
				imc->imloop(imc);
			}
			else if(imc->imloopcontinue<0)
				imc->imloop(imc);
		}

		p = imc->msglist;
		while(p != NULL)
		{
			ret = nob_write(imc->imfd, p->msg, p->len, imc->rtt);
			if(ret <= 0)
				break;
			p = p->next;

			lasttime = time(NULL);
		}

		p = imc->msglist;
		while( p != NULL) // free
		{
			p1 = p;
			free(p->msg);
			free(p);
			p = p1->next;
		}
		imc->msglist = NULL;

		currenttime = time(NULL);   //心跳机制，距离最近一次客户端向服务器发送的任何消息时长超过heartbeat时长则发送心跳包
		if(currenttime - lasttime >= imc->heartbeat)
		{
			lasttime = currenttime;
			//heartbeat
			im->session_id = s_id++;
			//write(fd, buf, 8);
			ret = nob_write(imc->imfd, buf, 8, imc->rtt);
			if(ret <= 0)
				return -1;
			printf("send alive...\n");
		}

		//sleep(1);  用读一秒钟替代延时1秒
		im_readcall(imc);
	}
	while(imc->run);

	//exit
	return -1;
}


void init_imc(imclib_t *imc)
{
	memset(imc, 0, sizeof(imclib_t));
	imc->heartbeat = 30;
	imc->port = 5200;
	imc->maxlen = 1000;
	imc->rtt = 1;
	imc->cb = NULL;
	imc->msglist = NULL;
	imc->run = 1;
	imc->imloop = NULL;
	imc->imloopcount = 10;
	imc->imloopcontinue = 1;
}

