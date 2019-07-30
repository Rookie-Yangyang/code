/*********************************************************************************
 *       Copyright:  (C) 2019 1434168279@qq.com
 *                   All rights reserved.
 *       
 *       Filename:   ssl-client.c
 *       Description:  This file 
 *                              
 *       Version:  1.0.0(24/07/19)
 *       Author:   1434168279@qq.com
 *       ChangeLog:  1, Release initial version on "24/07/19 16:30:00"
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
#include <getopt.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
 
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "init_socket.c"

#define MAXSIZE 1024

#define CA_CERT_FILE "ca.crt"
#define CLIENT_CERT_FILE "client.crt"
#define CLIENT_KEY_FILE "client.key"

int g_stop=0;
void sig_handler(int signum)/*signal信号处理函数*/
{
	if( SIGUSR1==signum )
	{
		g_stop=-1;
	}
}

/*打印证书信息*/
void ShowCerts(SSL * ssl)
{
	X509 *cert;
	char *line;

	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL)
	{
		printf("数字证书信息:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("证书: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("颁发者: %s\n", line);
		free(line);
		X509_free(cert);
	}
	else
		printf("无证书信息！\n");
}


void print_help(char *progname)
{
	printf("The progname is:%s\n",progname);
	printf("-i(--ipaddr):specify server ip!\n");
	printf("-p(--port):specify server port!\n");
	printf("-h(--help):print help information!\n");
	return ;
}

int main(int argc,char **argv)
{
	int                      ch;
	int                      len;
	int                      rv;
	int                      sockfd = -1;
	int                      server_port = 0;

	char                     send_buf[MAXSIZE];
	char                     rev_buf[MAXSIZE];
	char                     *server_ip = NULL;
	
	

	SSL_CTX                  *ctx;
	SSL                      *ssl;


	struct option opt[] = {
		{"ipaddr", required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	while ( (ch = getopt_long(argc, argv, "i:p:h", opt, NULL))!=-1 )
	{
		switch(ch)
		{
			case 'i':
				server_ip = optarg;
				break;
			case 'p':
				server_port = atoi(optarg);
				break;
			case 'h':
				print_help(argv[0]);
				return 0;
		}
	}


	if (!server_ip)
	{
		print_help(argv[0]);
		return 0;
	}

	/*SSL库初始化*/
	SSL_library_init();

	OpenSSL_add_all_algorithms();  
	SSL_load_error_strings();  
	ERR_load_BIO_strings();

	ctx = SSL_CTX_new(SSLv23_client_method());
	if (NULL == ctx)
	{
		printf("SSL_CTX_new error!\n");
		ERR_print_errors_fp(stdout);
		exit(1);
	}

	// 要求校验对方证书，表示需要验证服务器端，若不需要验证则使用  SSL_VERIFY_NONE
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);  

	// 加载CA的证书
	printf("SSL_CTX_load_verify_locations start!\n");
	if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
	{
		printf("SSL_CTX_load_verify_locations error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// 加载自己的证书  
	if(SSL_CTX_use_certificate_file(ctx, CLIENT_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_certificate_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	if(SSL_CTX_use_PrivateKey_file(ctx, CLIENT_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_PrivateKey_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	if(!SSL_CTX_check_private_key(ctx))
	{
		printf("SSL_CTX_check_private_key error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/*socket初始化*/
	sockfd=client_init(server_ip,server_port);

	/*创建SSL套接字*/
	ssl = SSL_new(ctx);
	if (NULL==ssl)
	{
		printf("SSL_new error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	SSL_set_fd(ssl, sockfd);

	if (SSL_connect(ssl) == -1)
	{
		printf("SSL_new error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}
	else
	{
		printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
		ShowCerts(ssl);
	}

	signal(SIGUSR1,sig_handler);

	while (!g_stop)
	{
		printf("Input message:\n");

		bzero(send_buf, MAXSIZE + 1);
		fgets(send_buf,MAXSIZE,stdin);

		len = SSL_write(ssl, send_buf, strlen(send_buf));
		if (len<0)
		{
			printf("消息'%s'发送失败！错误信息是'%s'\n", send_buf, strerror(errno));
			goto finish;
		}
		else
		{
			printf("消息%s发送成功，共发送了%d个字节！\n", send_buf, len-1);
		}

		
		memset(rev_buf, 0, sizeof(rev_buf));
		rv = SSL_read(ssl, rev_buf, sizeof(rev_buf));
		if (rv<0)
		{
			printf("read data from server failure:%s\n",strerror(errno));
			goto finish;
		}
		else
		{
			printf("read %zd bytes data from server is:%s\n",strlen(rev_buf)-1,rev_buf);
		}
	}

finish:
	SSL_shutdown(ssl);
	SSL_free(ssl);

	close(sockfd);
	SSL_CTX_free(ctx);


	return 0;
}
	
