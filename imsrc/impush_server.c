/*********************************************
license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
author:		arvik
email:		1216601195@qq.com
blog:		http://blog.csdn.net/u012819339
github:		https://github.com/arviklinux/impush
date:		2017.03.07
*********************************************/

#include <signal.h>
#include "im_prase.h"
#include "pt.h"
#include "im_epoll.h"
#include "im_ss.h"

extern server_status_t *p_imserver;

static int run_flag = 1;


void signalkill_handle(int sign)
{
	run_flag = 0;
	PT_RED("exit from singal kill!\n");
}



//this is server!
int main()
{
	int listenfd, epfd, connfd;
	socklen_t client;
	struct epoll_event events[200];
	
	struct sockaddr_in serveraddr;
	impush_conn_t listen_srv_c;

	signal(SIGINT, signalkill_handle);

	if(init_server_status(p_imserver) != NGX_OK)
	{
		PT_Fatal("init server failed\n");
		return -1;
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0)
	{
		PT_Fatal("create listen socket failed!\n");
		return -1;
	}

	ngx_nonblocking(listenfd);

	ngx_memzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("192.168.174.139");//htonl(INADDR_ANY); //inet_addr("42.96.130.249");
	serveraddr.sin_port = htons(5200);

	if(bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		PT_Fatal("server bind socket failed!, errno:%d\n", errno);
		close(listenfd);
		return -1;
	}

	if(listen(listenfd, 1000) < 0)
	{
		PT_Fatal("server set listen failed!, errno:%d\n", errno);
		close(listenfd);
		return -1;
	}


	listen_srv_c.fd = listenfd;
	listen_srv_c.sockaddr = serveraddr;
	listen_srv_c.socklen = sizeof(serveraddr);
	listen_srv_c.fdtype = 1;
	listen_srv_c.accept_handle = imconn_accept;
	listen_srv_c.imread = impush_read_data;


	if(im_epoll_init() != NGX_OK)
		return -1;

	im_epoll_add_connection(&listen_srv_c);
	
	while(run_flag)
	{
		im_epoll_process_events(1000);
		//printf("wait...\n");
		impush_spush_pump();
		impush_timeout_clear();
	}
	
	ak_epoll_done();
	return 0;

}





