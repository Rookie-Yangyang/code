/*********************************************************************************
 *      Copyright:  (C) 2019 LingYun<lingyun@email.com>
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  串口主函数
 *                 
 *        Version:  1.0.0(15/07/19)
 *         Author:  Wangjiayang
 *      ChangeLog:  1, Release initial version on "15/07/19 14:43:08"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <getopt.h>

#include "usart.c"
#include "usart.h"

void print_help(char *prog)
{
    printf("%s usage: \n",prog);
    printf("-D(--device):specify comport path\n");
    printf("-b(--baudrate):chose comport baudrate\n");
    printf("-d(--databit):chose comport databit\n");
    printf("-s(--stopbit):chose comport stopbit\n");
    printf("-q(--parity):chose comport parity\n");
    printf("-h(--help):print help information!\n");
    return ;
}

int main(int argc,char** argv)
{
    
    int                 ch;
    int                 rv;
    char                data_buf[64];
    char                rbuf[64];

    com_port            *comport_opt=InitComport();

    struct option opts[]={
        {"device",required_argument,NULL,'D'},
        {"baudrate",optional_argument,NULL,'b'},
        {"databit",optional_argument,NULL,'d'},
        {"stopbit",optional_argument,NULL,'s'},
        {"parity",optional_argument,NULL,'p'},
        {"help",no_argument,NULL,'h'},
        {NULL,0,NULL,0}
    };

    printf("term:%p\n",comport_opt);
    if( NULL == comport_opt  )
    {
        printf("term:%p\n",comport_opt);
        return -1;
    }

    while( (ch=getopt_long(argc, argv, "D:b::d::s::p::h", opts, NULL)) != -1 )
    {
        printf("ch=%c\n",ch);
        switch(ch)
        {
            case 'D':
                strncpy(comport_opt->path,optarg,strlen(optarg));
                printf("path:%s\n",comport_opt->path);
                break;
            case 'b':
                comport_opt->baudrate=atol(optarg);
                printf("baudrate:%ld\n",comport_opt->baudrate);
                break;
            case 'd':
                comport_opt->databit=atoi(optarg);
                printf("databit:%d\n",comport_opt->databit);
                break;
            case 's':
                comport_opt->stopbit=atoi(optarg);
                printf("stopbit:%d\n",comport_opt->stopbit);
                break;
            case 'p':
                comport_opt->parity=optarg[0];
                printf("parity:%c\n",comport_opt->parity);
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            default:
                print_help(argv[0]);
                return 0;
        }
    }

    if( comport_opt->path[0]==0 )
    {
        print_help(argv[0]);
        return 0;
    }

    if( USART_Open(comport_opt)<0 )
    {
        printf("USART_Open failure!\n");
        return 0;
    }

    printf("USART_Open ok!\n");

    while(1)
    {
        memset(data_buf,0,sizeof(data_buf));

        printf("Input data:\n");
        fgets(data_buf,64,stdin);

        if( USART_Send(comport_opt,data_buf,sizeof(data_buf))<0 )
        {
            printf("USART_Send failure!\n");
            break;
        }
        printf("USART_Send ok!\n");

        rv= USART_Read(comport_opt,rbuf,sizeof(rbuf));
        if( rv<0 )
        {
            printf("USART_Send failure!\n");
            break;
        }
        else if( rv==0 )
        {
            printf(" USART_Read timeout!\n");
            break;
        }

        printf("USART_Read ok!\n");

        
    }
    
    USART_Close(comport_opt);
    
    return 0;

}
