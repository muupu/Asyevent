#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <event.h>
#include <stdlib.h>

#define PORT        25341
#define BACKLOG     5
#define MEM_SIZE    1024

struct event_base* base;
struct sock_ev
{
    struct event* read_ev;
    struct event* write_ev;
    char* buffer;
};

void release_sock_event(struct sock_ev* ev)
{
    event_del(ev->read_ev);
    free(ev->read_ev);
    free(ev->write_ev);
    free(ev->buffer);
    free(ev);
}

void on_write(int sock, short event, void* arg)
{
    char* buffer = (char*)arg;
    send(sock, buffer, strlen(buffer), 0);

    free(buffer);
}

void on_read(int sock, short event, void* arg)
{
    struct event* write_ev;
    int size;
    struct sock_ev* ev = (struct sock_ev*)arg;
    ev->buffer = (char*)malloc(MEM_SIZE);
    bzero(ev->buffer, MEM_SIZE);
    size = recv(sock, ev->buffer, MEM_SIZE, 0);
    printf("receive data:%s, size:%d\n", ev->buffer, size);
    if (size == 0) 
    {
        release_sock_event(ev);
        close(sock);
        return;
    }
    event_set(ev->write_ev, sock, EV_WRITE, on_write, ev->buffer);
    event_base_set(base, ev->write_ev);
    event_add(ev->write_ev, NULL);
}

void on_accept(int sock, short event, void* arg)
{
    struct sockaddr_in cli_addr;
    int newfd, sin_size;
    struct sock_ev* ev = (struct sock_ev*)malloc(sizeof(struct sock_ev));
    ev->read_ev = (struct event*)malloc(sizeof(struct event));
    ev->write_ev = (struct event*)malloc(sizeof(struct event));
    sin_size = sizeof(struct sockaddr_in);
    newfd = accept(sock, (struct sockaddr*)&cli_addr, &sin_size);
    event_set(ev->read_ev, newfd, EV_READ|EV_PERSIST, on_read, ev);
    event_base_set(base, ev->read_ev);
    event_add(ev->read_ev, NULL);
}

// 参考http://www.cnblogs.com/cnspace/archive/2011/07/19/2110891.html
int main(int argc, char* argv[])
{
    struct sockaddr_in my_addr;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0); // 1. socket
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // SO_REUSEADDR是让端口释放后立即就可以被再次使用
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)); // 2. bind
    listen(sock, BACKLOG);  // 3. listen

    struct event listen_ev; 
    base = event_base_new(); //创建一个事件处理的全局变量,处理IO的管家
    // 在listen_ev这个事件监听sock这个描述字的读操作，当读消息到达是调用on_accept函数
    event_set(&listen_ev, sock, EV_READ|EV_PERSIST, on_accept, NULL);
    // 将listen_ev注册到base，相当于告诉处理IO的管家请留意我的listen_ev上的事件
    event_base_set(base, &listen_ev);
    // 告诉处理IO的管家，当有我的事件到达时你发给我(调用on_accept函数)，至此对listen_ev的初始化完毕
    event_add(&listen_ev, NULL);

    // 正式启动libevent的事件处理机制，使系统运行起来
    event_base_dispatch(base);

    return 0;
}

