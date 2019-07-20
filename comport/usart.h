/********************************************************************************
 *      Copyright:  (C) 2019 LingYun<lingyun@email.com>
 *                  All rights reserved.
 *
 *       Filename:  usart.h
 *    Description:  This head file is serial port .h
 *
 *        Version:  1.0.0(07/07/19)
 *         Author:  Wangjiayang
 *      ChangeLog:  1, Release initial version on "07/07/19 15:59:51"
 *                 
 ********************************************************************************/

#ifndef  _USART_H
#define  _USART_H

//串口相关头文件
#include<stdio.h>      /* 标准输入输出定义*/    
#include<stdlib.h>     /* 标准函数库定义*/    
#include<unistd.h>     /* Unix 标准函数定义*/    
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>      /* 文件控制定义*/    
#include<termios.h>    /* PPSIX 终端控制定义*/    
#include<errno.h>      /* 错误号定义*/    
#include<string.h>    

//宏定义    
#define FALSE  -1    
#define TRUE   0
#define MAX    64
#define DEV_NAME "ttyUSB0" 


typedef struct Comport{
    long baudrate;
    int databit;
    int stopbit;
    int fd;
    int isopen;
    char path[MAX];
    char parity;
}com_port;


com_port *InitComport();

void comport_term(com_port * comport);

int USART_Open(com_port * comport);

void USART_Set(com_port * comport);

int USART_Send(com_port * comport,char *write_buf,int w_datalen);

int USART_Read(com_port * comport,char *read_buf,int r_datalen);

void USART_Close(com_port * comport);

#endif
