/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include "imclib.h"


static char thismsgbuf[1024]={0};

int push_msg(struct imclib_s *imc)
{
	char *pstr = thismsgbuf;
	int16_t len = 0;

	len = strlen(pstr);

	return im_addmsg(pstr, len, 1, imc); //1：广播
}

//server push msg callback
int msg_cb(int8_t *msg, uint16_t msglen)
{
	printf("recv msg from server:%*s\n", msglen, msg);
	return 0;
}

int main(int argc, char *argv[])
{
	imclib_t imc;
	uint64_t info = 10;
	int oc;
	char ipbuf[24];
	int8_t mac[6];
	int set_info_flag=0;

	if(argc <= 1)
	{
		printf("usage:\nargs:\n");
		printf("-s :	server ip address str, eg. 42.96.130.233, you must set it!\n");
		printf("-p :	server listen port str, eg. 5200, default:5200\n");
		printf("-b :	heartbeat time x seconds, eg. 30, default: 30\n");
		printf("-i :	the unique info str for this dev, 8Bytes!, eg:acfb61gz, default: use eth0 mac for info!\n");
		printf("-m :	push the message to server, eg. hello world\n");
		printf("-c :	continue send message times, -1 means forever, 0 means no send, >0 means n times, deault: 1\n");
		printf("-t :	push message time interval, default: 10\n");
		return -1;
	}

	//memset(&imc, 0, sizeof(imclib_t));
	//imc.heartbeat = 30;
	//imc.port = 5200;
	//imc.maxlen = 1000;
	//imc.rtt = 1;
	//imc.cb = NULL;
	//imc.msglist = NULL;
	//imc.run = 1;

	init_imc(&imc);
//	imc.imloop = push_msg;

	memset(ipbuf, 0, sizeof(ipbuf));
	while((oc = getopt(argc, argv, "s:p:b:i:m:t:c:")) != -1)
	{
		switch(oc)
		{
			case 's':
				strcpy(imc.sipstr, optarg);
				imc.n_sip = inet_addr(imc.sipstr);
			break;
			case 'p':
				imc.port = atoi(optarg);
			break;
			case 'b':
				//printf("beat: %s\n", optarg);
				imc.heartbeat = atoi(optarg);
				//sscanf(optarg, "%u", &(imc.heartbeat));
			break;
			case 'i':
				memcpy(imc.info, optarg, 8);
				set_info_flag = 1;
			break;
			case 'm':
				strcpy(thismsgbuf, optarg);
				imc.imloop = push_msg;
			break;
			case 't':
				imc.imloopcontinue = atoi(optarg);
			break;
			case 'c':
				imc.imloopcount = atoi(optarg);
			break;
			default:
			break;
		}
	}

	if(set_info_flag != 1)
	{
		if(GetLocalDevMac0X("eth0", mac) != 0)
		{
			printf("get dev eth0 mac failed!\n");
			return -1;
		}
		memcpy(imc.info, mac, 6);
		imc.info[6] = 'b';
		imc.info[7] = 'm';
	}

	register_spush_cb(msg_cb, 0, &imc);//注册消息回调函数

	//if( im_connect("42.96.130.249", 5200, "abcdef80", &imc) != 0 )
	if( im_connect(&imc) != 0 )
	{
		printf("error\n");
		return -1;
	}

	im_aliveloop(&imc);

	return 0;
}