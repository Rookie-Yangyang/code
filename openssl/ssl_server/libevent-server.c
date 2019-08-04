#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <event.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#define CA_CERT_FILE "ca.crt"
#define SERVER_CERT_FILE "server.crt"
#define SERVER_KEY_FILE "server.key"



/*参数解析*/
void print_help(char *progname)
{
	printf("The project :%s\n",progname);
	printf("-p(--port):specify derver port\n");
	printf("-h(--help):print help information!\n");
}

/*socket初始化*/
int socket_init(char *server_ip,int server_port)
{
	int           sockfd;
	int           connfd;

	struct sockaddr_in    servaddr;


	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if( sockfd<0 )
	{
		printf("Create socket failure:%s\n",strerror(errno));
		return -1;
	}
	printf("Create socket successfully!\n");

	/*允许多次绑定同一个地址。要用在socket和bind之间*/
	/*evutil_make_listen_socket_reuseable(sockfd);*/

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(server_port);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	evutil_make_socket_nonblocking(sockfd);

	if( (bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)))<0 )
	{
		printf("bind failure!\n");
		return -2;
	}

	listen(sockfd,64);

	/*跨平台统一接口，将套接字设置为非阻塞状态*/
	/*evutil_make_socket_nonblocking(sockfd);*/
	return sockfd;
}

/*回调函数read_cb*/
void read_cb(int clifd, short events, void* arg)
{
	SSL* ssl = (SSL*)arg;

	char        buf[1024];

	size_t      rv;

	memset(buf,0,sizeof(buf));
	rv=SSL_read(ssl, buf, sizeof(buf)); 

	printf("Read data from client is:%s\n",buf);

	char reply_buf[1024] = "I have recvieced the msg: ";

	strcat(reply_buf + strlen(reply_buf), buf);
	SSL_write(ssl, reply_buf, strlen(reply_buf));

}

SSL* CreateSSL(int clifd)
{   
	SSL_CTX* ctx = SSL_CTX_new (SSLv23_server_method());
	if( ctx == NULL)
	{
		printf("SSL_CTX_new error!\n");
		return NULL;
	}

	/* 要求校验对方证书  */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);  

	/* 加载CA的证书  */
	if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
	{
		printf("SSL_CTX_load_verify_locations error!\n");
		return NULL;
	}

	/* 加载自己的证书  */
	if(SSL_CTX_use_certificate_file(ctx, SERVER_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_certificate_file error!\n");
		return NULL;
	}

	/* 加载自己的私钥  */
	if(SSL_CTX_use_PrivateKey_file(ctx, SERVER_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_PrivateKey_file error!\n");
		return NULL;
	}

	/*检查私钥是否正确*/
	if(!SSL_CTX_check_private_key(ctx))
	{
		printf("SSL_CTX_check_private_key error!\n");
		return NULL;
	}

	/*申请ssl套接字*/
	SSL *ssl = SSL_new (ctx);
	if(!ssl)
	{
		printf("SSL_new error!\n");
		return NULL;
	}

	if ( (SSL_set_fd (ssl, clifd)) != 1 )
	{
		printf("SSL_set_fd error!\n");
		return 0;
	}

	
	if( (SSL_accept (ssl)) != 1)
	{
		int icode = -1;
		int iret = SSL_get_error(ssl, icode);
		ERR_print_errors_fp(stderr);
		printf("SSL_accept error:%s! code = %d, iret = %d\n", strerror(errno), icode, iret);
		return NULL;
	}

	return ssl;
}

void do_accept(int listenfd, short event, void *arg)
{
	printf("do_accept\n");
	
	struct sockaddr_in      cliaddr;
	evutil_socket_t         clifd;
	socklen_t               len=sizeof(cliaddr);

	clifd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
	if (clifd < 0)
	{
		perror("accept");
	}
	else if (clifd > FD_SETSIZE)
	{
		close(clifd);
	}
	else
	{
		printf("Accept new client[%d] successfully!\n",clifd);

		SSL* ssl = CreateSSL(clifd);
		
		struct event_base* base = (struct event_base*)arg;
		
		struct event* cliev = event_new(NULL, -1, 0, NULL, NULL);

		//将动态创建的结构体作为event的回调参数
		event_assign(cliev, base, clifd, EV_READ | EV_PERSIST, read_cb, (void*)ssl);

		event_add(cliev, NULL);
	}
}


int main(int argc,char **argv)
{
	int                       ch;
	int                       port = 0;
	int                       listenfd = -1;


	struct option opt[]={
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while( (ch=getopt_long(argc,argv,"p:h",opt,NULL))!=-1 )
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
	printf("port:%d\n",port);

	if( !port )
	{
		print_help(argv[0]);
		return 0;
	}

	/*SSL初始化*/
	SSL_library_init();

	/*载入所有算法和错误信息*/
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();


	/*socket初始化*/
	listenfd=socket_init(NULL,port);

	if( listenfd<0 )
	{
		printf("socket_init failure!\n");
		return -1;
	}
	printf("socket_init successfully!\n");

	struct event_base *base = event_base_new();
	if (!base)
	{
		return -1;
	}

	struct event *listener_event = event_new(base, listenfd, EV_READ|EV_PERSIST, do_accept, (void*)base);
	event_add(listener_event, NULL);
	/*启动事件循环*/
	event_base_dispatch(base);

	return 0;
}
