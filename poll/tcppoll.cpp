#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>

// ulimit -n 最大文件描述符
#define MAXNFDS 1024

int initserver(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // Linux如下
    int opt = 1;
    unsigned int len = sizeof(opt);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, len);
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, len);

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
        printf("Using ./tcppoll port\n");
        return -1;
    }
    int listensock = initserver(atoi(argv[1]));
    printf("listen port :%s,sock=%d\n", argv[1], listensock);

    // poll需要fd的数组
    int maxfd;
    struct pollfd fds[MAXNFDS];
    for (int i = 0; i < MAXNFDS; ++i) {
        // 初始化-1
        fds[i].fd = -1;
    }
    // listensock加入fds
    fds[listensock].fd = listensock;
    fds[listensock].events = POLLIN;
    maxfd = listensock;

    while (true) {
        int nfds = poll(fds, maxfd + 1, 100000);
        if (nfds < 0) {
            printf("poll fail\n");
            break;
        }
        if (nfds == 0) {
            printf("no event | timeout \n");
            continue;
        }
        // 有事件来了
        // 跟select一样,需要遍历全部
        for (int eventfd = 0; eventfd <= maxfd; ++eventfd) {
            if (fds[eventfd].fd == -1) continue;
            // 只处理读事件
            if ((fds[eventfd].revents & POLLIN) == 0) continue;

            // 清空revents
            fds[eventfd].revents = 0;

            // listen的fd
            if (eventfd == listensock) {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(listensock, (struct sockaddr *) &client, &len);
                if (clientfd < 0) {
                    printf("accept fail\n");
                    continue;
                }
                if (clientfd > MAXNFDS) {
                    printf("reach MAXNFDS\n");
                    close(clientfd);
                    continue;
                }
                printf("client(%s %d) socket=%d connected ok\n",
                       inet_ntoa(client.sin_addr), client.sin_port, clientfd);
                fds[clientfd].fd = clientfd;
                fds[clientfd].events = POLLIN;
                fds[clientfd].revents = 0;
                if (clientfd > maxfd) {
                    maxfd = clientfd;
                }
                printf("maxfd:%d\n", maxfd);
                continue;
            } else {
                // 客户端fd
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                int isize;
                isize = read(eventfd, buffer, sizeof(buffer));
                if (isize <= 0) {
                    printf("client socket=%d disconnected ok", eventfd);
                    close(eventfd);
                    fds[eventfd].fd = -1;
                    if (eventfd == maxfd) {
                        for (int i = maxfd; i > 0; --i) {
                            if (fds[i].fd != -1) {
                                maxfd = i;
                                break;
                            }
                        }
                        printf("maxfd:%d\n", maxfd);
                    }
                } else {
                    printf("recv(eventfd=%d,size=%d):%s\n", eventfd, isize, buffer);
                    write(eventfd, buffer, sizeof(buffer));
                }
            }
        }
    }
}
