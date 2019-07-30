/*********************************************************************************
 *  *         Copyright:  (C) 2019 1434168279@qq.com
 *                        All rights reserved.
 *           
 *            Filename:  ssl-server.c
 *            Description:  This file 
 *                                     
 *            Version:  1.0.0(21/07/19)
 *            Author:  
 *            ChangeLog:  1, Release initial version on "21/07/19 09:54:47"
 *                                      
 **********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include "init_socket.c"

#define MAXSIZE 1024

#define CA_CERT_FILE "ca.crt"
#define SERVER_CERT_FILE "server.crt"
#define SERVER_KEY_FILE "server.key"

int g_stop=0;
void sig_handler(int signum)
{
	if( SIGUSR1==signum )
	{
		g_stop=-1;
	}
}

void print_help(char *progname)
{
	printf("The progname is:%s\n",progname);
	printf("-p(--port):specify listen port!\n");
	printf("-h(--help):print help information!\n");
	return ;
}

int main(int argc, char **argv)
{
	int                   ch;
	int                   port = 0;
	int                   len = 0;
	int                   rv = 0;
	int                   clifd = -1;
	int                   sock_fd = -1;

	char                  send_buf[MAXSIZE];
	char                  rev_buf[MAXSIZE];

	SSL_CTX               *ctx;
	SSL                   *ssl;

	struct option opt[] = {
		{"port", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	while ( (ch = getopt_long(argc, argv, "p:h", opt, NULL))!=-1 )
	{
		switch(ch)
		{
			case 'p':
				port=atoi(optarg);
				break;
			case 'h':
				print_help(argv[0]);
				return 0;
		}
	}

	if (!port)
	{
		print_help(argv[0]);
		return 0;
	}

	/*错误队列*/
	ERR_load_BIO_strings();

	/*SSL初始化*/
	SSL_library_init();
	printf("SSL_library_init ok!\n");

	OpenSSL_add_all_algorithms();  
	SSL_load_error_strings();  
	ERR_load_BIO_strings();  

	/*建立会话环境*/
	printf("建立会话环境....\n");

	ctx = SSL_CTX_new(SSLv23_server_method());
	if (ctx == NULL)
	{
		printf("建立会话环境失败!\n");
		ERR_print_errors_fp(stdout);
		exit(1);
	}

	// 是否要求校验对方证书 此处不验证客户端身份所以为： SSL_VERIFY_NONE
	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL); 

	// 加载CA的证书  
	if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
	{
		printf("SSL_CTX_load_verify_locations error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// 加载自己的证书  
	if(SSL_CTX_use_certificate_file(ctx, SERVER_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_certificate_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/* 加载自己的私钥  私钥的作用是，ssl握手过程中，对客户端发送过来的随机
	 * 消息进行加密，然后客户端再使用服务器的公钥进行解密，若解密后的原始消息跟
	 * 客户端发送的消息一直，则认为此服务器是客户端想要链接的服务器*/
	if(SSL_CTX_use_PrivateKey_file(ctx, SERVER_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_PrivateKey_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}
	

	/*检查用户私钥是否正确*/
	if (!SSL_CTX_check_private_key(ctx))
	{
		printf("SSL_CTX_check_private_key error!\n");
		ERR_print_errors_fp(stdout);
		return -1;
	}

	/*socket初始化*/
	sock_fd = server_init(NULL, port);
	if (sock_fd<0)
	{
		printf("socket_init failure!\n");
		return -1;
	}

	printf("socket_init ok!\n");

	clifd = accept(sock_fd, (struct sockaddr *)NULL, NULL);
	if (clifd<0)
	{
		printf("accept new client failure:%s\n",strerror(errno));
		return -1;
	}
	printf("Accept new client[%d] successfully!\n",clifd);

	/*申请SSL套接字*/
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, clifd);

	if (SSL_accept(ssl) == -1)
	{
		printf("SSL_accept failure:%s!\n",strerror(errno));
		close(clifd);
		return -2;
	}

	signal(SIGUSR1,sig_handler);

	while (!g_stop)
	{
		//printf("start to accept new client incoming...\n");

		memset(rev_buf, 0,sizeof(rev_buf));
		len = SSL_read(ssl,rev_buf, MAXSIZE);
		if (len > 0)
		{
			printf("接收消息成功:%s，共%d个字节的数据\n",rev_buf, len-1);
		}
		else
		{
			printf("消息接收失败！错误信息是'%s'\n", strerror(errno));
		}

		
		memset(send_buf,0,sizeof(send_buf));
		printf("Input reply message:\n");
		fgets(send_buf,MAXSIZE,stdin);

		rv = SSL_write(ssl, send_buf, MAXSIZE);
		if (rv>0)
		{
			printf("send %ld bytes data to client:%s\n", strlen(send_buf)-1, send_buf);
		}
		else
		{	
			printf("send data to client failure:%s\n", strerror(errno));	
			break;
		}
	}

	SSL_shutdown(ssl);
	SSL_free(ssl);
	close(clifd);

	close(sock_fd);
	SSL_CTX_free(ctx);
	return 0;

}

