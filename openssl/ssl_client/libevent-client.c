#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <event.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>

#define CA_CERT_FILE "ca.crt"
#define CLIENT_CERT_FILE "client.crt"
#define CLIENT_KEY_FILE "client.key"

void print_help(char *progname)
{       
	printf("The project :%s\n",progname);
	printf("-i(--ipaddr):specify server ip\n");
	printf("-p(--port):specify derver port\n");
	printf("-h(--help):print help information!\n");
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

/*socket 初始化*/
int socket_connect(char *server_ip,int server_port)
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

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(server_port);
	inet_aton(server_ip,&servaddr.sin_addr);
	connfd=connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if( connfd<0 )
	{
		printf("Connect to server failure:%s\n",strerror(errno));
		return -2;
	}
	printf("Connect to server successfully!\n");
	/*设置为非阻塞*/
	evutil_make_socket_nonblocking(sockfd);

	return sockfd;
}

/*终端输入*/
void input_cb(int fd,short events,void* arg)
{
	char      buf[1024];
	int       len;
	SSL*      ssl = (SSL*)arg;

	len=read(fd,buf,sizeof(buf));
	if( len<0 )
	{
		printf("Read failure!\n");
		exit(1);
	}

	struct bufferevent* bev=(struct bufferevent*)arg;

	SSL_write(ssl,buf,len);
}

/*回调函数*/
void read_cb(int fd, short events, void *arg)
{
	SSL*    ssl = (SSL*)arg;
	char    buf[1024];
	int     rv;

	memset(buf, 0, sizeof(buf));
	rv=SSL_read(ssl,buf,sizeof(buf));
	if( rv<=0 )
	{
		printf("Read data from server failure!\n");
	}

	printf("Read data from server is:%s\n",buf);
}

int main(int argc, char **argv)
{
	int                         ch;
	int                         sockfd = -1;
	int                         server_port = 0;

	char                        *server_ip = NULL;
	
	SSL_CTX* ctx = NULL;  

	struct option opt[]={
		{"server_ip",required_argument,NULL,'i'},
		{"server_port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};


	while( (ch=getopt_long(argc,argv,"i:p:h",opt,NULL))!=-1 )
	{
		switch(ch)
		{
			case 'i':
				server_ip=optarg;
				break;
			case 'p':
				server_port=atoi(optarg);
				break;
			case 'h':
				print_help(argv[0]);
				return 0;
		}
	}

	if(!server_port)
	{
		print_help(argv[0]);
		return 0;
	}

	/*SSL库初始化*/
	if (SSL_library_init() == 1 )
	{
		printf("SSL_library_init() ok!\n");
	}

	/*加载所有的算法及错误信息*/
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ERR_load_BIO_strings();

	/*建立会话环境*/
	ctx = SSL_CTX_new(SSLv23_client_method());
	if ( ctx == NULL )
	{
		ERR_print_errors_fp(stdout);
		SSL_CTX_free (ctx);
		exit(1);
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	/*加载CA证书*/
	printf("SSL_CTX_load_verify_locations start!\n");
	if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
	{
		printf("SSL_CTX_load_verify_locations error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/*加载客户端证书*/
	if(SSL_CTX_use_certificate_file(ctx, CLIENT_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_certificate_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/*加载客户端私钥*/
	if(SSL_CTX_use_PrivateKey_file(ctx, CLIENT_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_PrivateKey_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/*检查私钥是否正确*/
	if(!SSL_CTX_check_private_key(ctx))
	{
		printf("SSL_CTX_check_private_key error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/*socket初始化*/
	sockfd = socket_connect(server_ip, server_port);
	if (sockfd < 0)
	{
	printf("socket_connect failure!\n");
	return -1;
	}

	printf("创建ssl套接字\n");
	

	/*int           connfd;

	struct sockaddr_in    servaddr;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if( sockfd<0 )
	{
		printf("Create socket failure:%s\n",strerror(errno));
		return -1;
	}
	printf("Create socket successfully!\n");

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(server_port);
	inet_aton(server_ip,&servaddr.sin_addr);
	connfd=connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if( connfd<0 )
	{
		printf("Connect to server failure:%s\n",strerror(errno));
		return -2;
	}
	printf("Connect to server successfully!\n");
	
	*设置为非阻塞*
	evutil_make_socket_nonblocking(sockfd);*/


	/*创建SSL套接字*/
	SSL *ssl = SSL_new(ctx);
	printf("address:%p\n",ssl);
	if (ssl == NULL)
	{
		printf("SSL_new error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	SSL_set_fd(ssl, sockfd);

	if (SSL_connect(ssl) == -1)
	{
		printf("SSL_connect filure:%s!\n", strerror(errno));
		ERR_print_errors_fp(stderr);
		return -1;
	}
	else
	{
		printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
		ShowCerts(ssl);
	}

	struct event_base* base = event_base_new();

	/*读事件*/
	struct event *rev_event = event_new(base, sockfd,  EV_READ | EV_PERSIST,  read_cb, (void*)ssl);
	event_add(rev_event, NULL);

	/*写事件*/
	struct event* ev_input = event_new(base, STDIN_FILENO,  EV_READ | EV_PERSIST, input_cb,  (void*)ssl);
	event_add(ev_input, NULL);

	/*启动事件循环*/
	event_base_dispatch(base);

	return 0;
}
