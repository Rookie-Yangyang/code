/*********************************************************************************
 *         Copyright:  (C) 2019 1434168279@qq.com
 *                     All rights reserved.
 *       
 *         Filename:  init_socket.c
 *         Description:  This file 
 *                              
 *         Version:  1.0.0(21/07/19)
 *         Author:  
 *         ChangeLog:  1, Release initial version on "21/07/19 09:54:47"
 *                                    
 *********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

/* 服务器端*/
int server_init(char *server_ip,int server_port)
{
	int           sockfd;
	int           on=1;

	struct sockaddr_in    servaddr;


	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if( sockfd<0 )
	{
		printf("Create socket failure:%s\n", strerror(errno));
		return -1;
	}
	printf("Create socket successfully!\n");

	/* 允许多次绑定同一个地址。要用在socket和bind之间*/
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(server_port);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	if( (bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)))<0 )
	{
		printf("bind failure!\n");
		return -2;
	}

	listen(sockfd,64);
	printf("Start to listen port[%d]\n",server_port);

	return sockfd;
}

/* 客户端 */
int client_init(char *server_ip,int server_port)
{
	int           sockfd;
	int           connfd;

	struct sockaddr_in    servaddr;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if( sockfd<0  )
	{
		printf("Create socket failure:%s\n",strerror(errno));
		return -1;

	}
	printf("Create socket successfully!\n");

	printf("server address=%s,server port=%d\n",server_ip,server_port);
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(server_port);
	inet_aton(server_ip,&servaddr.sin_addr);

	connfd=connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));


	if( connfd<0  )
	{
		printf("Connect to server failure:%s\n",strerror(errno));
		return -2;

	}
	printf("Connect to server successfully!\n");


	return sockfd;
}
