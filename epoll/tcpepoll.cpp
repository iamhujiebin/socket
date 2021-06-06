#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

// ulimit -n 最大文件描述符
#define MAXNFDS 1024

int initserver(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    bind(sock, (sockaddr *) &servaddr, sizeof(servaddr));
    listen(sock, 5);
    return sock;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Using ./tcpepoll port\n");
        return -1;
    }
    // 初始化服务端用于监听的socket。
    int listensock = initserver(atoi(argv[1]));
    printf("listensock=%d\n", listensock);

    if (listensock < 0) {
        printf("initserver() failed.\n");
        return -1;
    }
    // epoll需要创建自己的fd
    int epollfd;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    epollfd = epoll_create(1);
}
