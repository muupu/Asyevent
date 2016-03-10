
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <sys/time.h>

int test_getsockopt()
{
    int fds[2];
  struct ucred cred;
  socklen_t size;

  if(socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) < 0) {
    perror("socketpair");
    return 1;
  }

  fprintf(stderr, "process:  pid=%i uid=%i gid=%i\n",
      getpid(), getuid(), getgid());

  size = sizeof(cred);
  if(getsockopt(fds[1], SOL_SOCKET, SO_PEERCRED, &cred, &size) < 0) {
    perror("getsockopt/SO_PEERCRED");
    return 1;
  }
  if(size == sizeof(cred)) {
    fprintf(stderr, "socket:   pid=%i uid=%i gid=%i\n",
        cred.pid, cred.uid, cred.gid);

    if(getpid() == cred.pid &&
       getuid() == cred.uid &&
       getgid() == cred.gid) {
      printf("IDs are the same, as expected\n");
      return 0;
    }
    else {
      printf("IDs differ\n");
    }
  }
  return 1;
}

int main(int argc, char *argv[])
{
    test_getsockopt();

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 
    struct ucred cred;
    socklen_t  len;

    char sendBuff[1025];
    time_t ticks; 

    struct timeval start_tv, end_tv;

    /* daveti: init cred */
    memset(&cred, 0x0, sizeof(cred));
    len = sizeof(cred);

    /* daveti: display our pid */
    printf("TCP server pid [%u]\n", getpid());

    listenfd = socket(PF_UNIX, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

      	/* daveti: retrieve the peer cred */
      	gettimeofday(&start_tv, NULL);
      	if (getsockopt(connfd, SOL_SOCKET, SO_PEERCRED, (void*)&cred, &len))
      		printf("Error: getsockopt failed with error %s\n",
      			strerror(errno));
      	else
      		printf("Client pid/uid/gid [%u/%u/%u]\n",
      			cred.pid, cred.uid, cred.gid);
      	gettimeofday(&end_tv, NULL);
      	printf("socket2pid query time [%lu] us\n",
      		((end_tv.tv_sec-start_tv.tv_sec)*1000000 +
      		(end_tv.tv_usec-start_tv.tv_usec)));

        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(connfd, sendBuff, strlen(sendBuff)); 

        close(connfd);
        sleep(1);
     }
}