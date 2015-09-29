#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#define cerror(str) do{perror(str); exit(EXIT_FAILURE); }while(0)

static int port; 
static char addr[256];
static int nlen;
static char msg[]="Hello, I'm client.";

// "./client localhost 25341"
int main(int argc, char*argv[])
{
    
    int fd;
    struct sockaddr_in sin; // Socket套接字地址(参考http://blog.sina.com.cn/s/blog_6444798b0100n529.html)
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET; // 协议簇为（TCP/IP协议传输 – IPv4）

    char *recv_msg = (char *)malloc(256 * sizeof(char));
  
    if(argc<3)
    {
        port = 8888;
    }else
    {
        port = atoi(argv[2]);
    }
    
    sin.sin_port = htons(port); // 端口号(htons()将一个数的高低位互换,将一个16位数从主机字节顺序转换成网络字节顺序)
    
    if(argc<2)
    {
        strcpy(addr, argv[1]);
        if (inet_pton(AF_INET, addr, &(sin.sin_addr) )<0) // inet_pton转换字符串到in_addr的结构体, 32位的IP地址
        {
             struct hostent *psh;  
             psh=gethostbyname(addr); // 用域名或主机名获取IP地址
             if(psh!=NULL)
                inet_pton(AF_INET, psh->h_addr, &(sin.sin_addr) );
             else
                cerror("inet_pton");
        }
    }
    
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0) //当套接字创建成功时，返回套接字。protocol一般设置为“0”
        cerror("socket");
    
    if(connect(fd, (struct sockaddr*)&sin, sizeof(sin))<0 ) // TCP建立连接
        cerror("sonnect");
    
    if( (nlen = write(fd,msg, strlen(msg)))<0 )  // 发送“hello\n”
        cerror("write");
        
    if( (nlen = read(fd,recv_msg, strlen(recv_msg)))<0 )  // 服务器回应信息
        cerror("read");
    recv_msg[nlen]='\0';
    printf("msg: %s\n", recv_msg);
    return 0;
}

