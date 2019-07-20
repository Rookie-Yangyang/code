/*********************************************************************************
 *      Copyright:  (C) 2019 LingYun<lingyun@email.com>
 *                  All rights reserved.
 *
 *       Filename:  usart.c
 *    Description:  串口设置
 *                 
 *        Version:  1.0.0(07/07/19)
 *         Author:  Wangjiayang
 *      ChangeLog:  1, Release initial version on "07/07/19 16:44:43"
 *                 
 ********************************************************************************/

#include "usart.h"


struct termios newopt,oldopt;


/* 串口参数默认值 */
com_port *InitComport()
{
    com_port *comport_opt=NULL;

    if( (comport_opt=(com_port*)malloc(sizeof(com_port)))==NULL )
    {
        return NULL;
    }

    memset(comport_opt,0,sizeof(comport_opt));
    
    //strncpy(comport_opt->path, DEV_NAME, sizeof(DEV_NAME));
    //printf("device:%s\n",comport_opt->path);


    comport_opt->parity='N';
    comport_opt->databit=8;
    comport_opt->stopbit=1;
    comport_opt->baudrate=115200;
    comport_opt->fd=-1;
    comport_opt->isopen=0;
    
    return comport_opt;
}

void comport_term(com_port * comport)
{
    if(NULL == comport)
    {
        return;
    }

    if (0 != comport->fd)
    {
        USART_Close(comport);
    }

    memset(comport, 0, sizeof(com_port)); 
    free(comport);
    comport = NULL;

    return;

}

/* 设置奇偶校验 */
int set_parity(char parity)
{
    switch(parity)
    {
        case 'n':
        case 'N':/*  无奇偶校验 */
            newopt.c_cflag&= ~PARENB;
            newopt.c_iflag&= ~INPCK;
            break;
        case 'o':
        case 'O':/*  奇校验 */
            newopt.c_cflag|= (PARODD | PARENB);
            newopt.c_iflag|= INPCK;
            break;
        case 'e':
        case 'E':/*  偶校验 */
            newopt.c_cflag|= PARENB;
            newopt.c_cflag&= ~PARODD;
            newopt.c_iflag|= INPCK;
            break;
        case 's':
        case 'S':/*  设置为空格 */
            newopt.c_cflag&= ~PARENB;
            newopt.c_cflag&= ~CSTOPB;
            break;
        default:
            printf("Unsupported parity\n");
            return -1;
    }
}

/* 设置停止位 */
int set_stopbit(int stopbit)
{
    switch(stopbit)
    {
        case 1:
            newopt.c_cflag&= ~CSTOPB;
            break;
        case 2:
            newopt.c_cflag|= CSTOPB;
            break;
        default:
            printf("Unsupported stopbit\n");
            return -1;
    }
}

/* 设置数据位 */
int set_databit(int databit)
{
    
    switch(databit)
    {
        case 5:
            newopt.c_cflag|=CS5;
            break;
        case 6:
            newopt.c_cflag|=CS6;
            break;
        case 7:
            newopt.c_cflag|=CS7;
            break;
        case 8:
            newopt.c_cflag|=CS8;
            break;
        default:
            printf("Unknow databit \n");
            return -1;
    }
}
/* 设置波特率 */
void set_baudrate(long baudrate)
{
    /* 0 50 75 110 134 150 200 300 600 1200 1800 2400 4800 9600 19200 38400 57600 115200 230400 */
    

    switch(baudrate)
    {
        case 0:
            cfsetispeed(&newopt,B0);
            cfsetospeed(&newopt,B0);
            break;
        case 50:
            cfsetispeed(&newopt,B50);
            cfsetospeed(&newopt,B50);
            break;
        case 110:
            cfsetispeed(&newopt,B110);
            cfsetospeed(&newopt,B110);
            break;
        case 134:
            cfsetispeed(&newopt,B134);
            cfsetospeed(&newopt,B134);
            break;
        case 150:
            cfsetispeed(&newopt,B150);
            cfsetospeed(&newopt,B150);
            break;
        case 200:
            cfsetispeed(&newopt,B200);
            cfsetospeed(&newopt,B200);
            break;
        case 300:
            cfsetispeed(&newopt,B300);
            cfsetospeed(&newopt,B300);
            break;
        case 600:
            cfsetispeed(&newopt,B600);
            cfsetospeed(&newopt,B600);
            break;
        case 1200:
            cfsetispeed(&newopt,B1200);
            cfsetospeed(&newopt,B1200);
            break;
        case 1800:
            cfsetispeed(&newopt,B1800);
            cfsetospeed(&newopt,B1800);
            break;
        case 2400:
            cfsetispeed(&newopt,B2400);
            cfsetospeed(&newopt,B2400);
            break;
        case 4800:
            cfsetispeed(&newopt,B4800);
            cfsetospeed(&newopt,B4800);
            break;
        case 9600:
            cfsetispeed(&newopt,B9600);
            cfsetospeed(&newopt,B9600);
            break;
        case 19200:
            cfsetispeed(&newopt,B19200);
            cfsetospeed(&newopt,B19200);
            break;
        case 38400:
            cfsetispeed(&newopt,B38400);
            cfsetospeed(&newopt,B38400);
            break;
        case 57600:
            cfsetispeed(&newopt,B57600);
            cfsetospeed(&newopt,B57600);
            break;
        case 115200:
            cfsetispeed(&newopt,B115200);
            cfsetospeed(&newopt,B115200);
            break;
        case 230400:
            cfsetispeed(&newopt,B230400);
            cfsetospeed(&newopt,B230400);
            break;
        default:
            cfsetispeed(&newopt,B9600);
            cfsetospeed(&newopt,B9600);
            break;
    }
}


void USART_Set(com_port * comport)
{
    
    set_stopbit(comport->stopbit);
    
    set_parity(comport->parity);
    

    set_databit(comport->databit);
    
    set_baudrate(comport->baudrate);
    
}


/* **********************************
 * *功能：打开串口
 ***********************************/
int USART_Open(com_port *comport)
{

    if( NULL==comport )
    {
        return -1;
    }
    
    comport->fd = open(comport->path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if( comport->fd<0 )
    {
        printf("Open device [%s] failure...\n", comport->path);
        return -2;
    }

    printf("Open device \"%s\"\n", comport->path);

    if( -1 == fcntl(comport->fd, F_GETFL, 0) )
    {
        /* 读取文件状态标志 */
        printf("fcntl failure!\n");
        
    }

        
    if( -1 != fcntl(comport->fd, F_SETFL, ~O_NONBLOCK) )
    {
        /* 设置文件状态为非阻塞 */
        if (-1 == tcflush(comport->fd, TCIOFLUSH))
        {
            /* 清空输入、输出缓存 */
            printf("tcflush failure!\n");
            return -3;
            
        }
    }
    else                  
    {
        printf("Open device ok!\n");
    }

    /* 获取相关参数 */
    if (0 != tcgetattr(comport->fd, &oldopt))
    {
        printf("Open device failure!");
        return -4;
    }

    memset(&newopt, 0, sizeof(newopt));

    newopt.c_cflag &= ~CSIZE;
    newopt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newopt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    newopt.c_oflag &= ~(OPOST);

    USART_Set(comport);

    /*  设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/
    newopt.c_cc[VTIME] = 0;/*  非规范模式读取时的超时时间；*/
    newopt.c_cc[VMIN]  = 0; /*  非规范模式读取时的最小字符数*/

    /* tcflush清空终端未完成的输入输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */ 
    tcflush(comport->fd ,TCIFLUSH);

    if (0 != tcsetattr(comport->fd, TCSANOW, &newopt))
    {
        printf("set device comport failure!\n");   
        return -5;
    }

    printf("device:%s ok!\n",comport->path);

    comport->isopen=1;

    return comport->fd;
}

/* ****************************************
 * *功能：通过串口发数据
 * ****************************************/
int USART_Send(com_port * comport,char *write_buf,int w_datelen)
{
    ssize_t     count=0;

    if( write_buf==NULL || w_datelen<=0 )
    {
        printf("error\n");
        return -1;
    }
    if( comport->isopen!=1 )
    {
        printf("comport is not open!\n");
        return -2;
    }

    count=write(comport->fd,write_buf,w_datelen);
    if( count==-1 )
    {
        printf("Write error!\n");
        return -3;
    }

    printf("write ok!\n");
    return count;
}

/* **********************************************************
 * *功能：接收串口数据
 * **********************************************************/
int USART_Read(com_port *comport,char *read_buf,int r_datelen)
{
    ssize_t           count=0;
    fd_set            rfds;//关注是否存在待读取数据的文件描述符注册到fd_set变量
    fd_set            exceptfds;//关注是否发生异常的文件描述符注册到fd_set变量
    struct timeval    time;

    int               ret;

    if( read_buf==NULL || r_datelen<=0 )
    {
        printf("error....\n");
        return -1;
    }

    if( comport->isopen!=1 )
    {
        printf("comport is not open...\n");
        return -2;
    }

    /*  将文件描述符加入读描述符集合 */
    FD_ZERO(&exceptfds);
    FD_ZERO(&rfds);
    FD_SET(comport->fd,&rfds);
    FD_SET(comport->fd,&exceptfds);

    /*  设置超时为15s*/
    time.tv_sec = 15;
    time.tv_usec = 0;
    /*  实现串口的多路IO */
    ret=select(comport->fd+1,&rfds,0,&exceptfds,&time);
    
    if( ret==0 )
    {
        printf("no data in read_buf...\n");
        return 0;
    }

    else if( ret<0 )
    {
        if (0 != FD_ISSET(comport->fd, &exceptfds) )
        {
            printf("select error...\n");
            return -3;
        }

        if( 0 == FD_ISSET(comport->fd, &rfds) )
        {
            printf("no data incoming....\n");
            return 0;
        }
    }

    else if( ret>0 )
    {
        sleep(1);/* wait data incoming */

        count=read(comport->fd,read_buf,r_datelen);

        if( count<=0 )
        {
            printf("read error...\n");
            return -4;
        }
        else
        {
            printf("read %d bytes data is:%s\n",strlen(read_buf)-1,read_buf);
            return count;
        }
    }


} 
/* *
 * *功能：关闭串口
 * */
void USART_Close(com_port * comport)
{
    if (0 != comport->fd)
    {
        printf("Close device \"%s\"\n", comport->path);
        close(comport->fd);
    }

    comport->isopen = 0;
    comport->fd = -1;
    
}
