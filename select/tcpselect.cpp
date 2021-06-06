#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

// 初始化服务端的监听端口
// @return 返回listen的sock
int initserver(int port);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Using ./tcpselect port\n");
        return -1;
    }
    int listensock = initserver(atoi(argv[1]));
    if (listensock < 0) {
        printf("listen fail\n");
        return -1;
    }

    // 读事件的fd集合,listenfd+clientfd
    fd_set readfdset;
    int maxfd;

    FD_ZERO(&readfdset);
    FD_SET(listensock, &readfdset);
    maxfd = listensock;

    while (true) {
        // select会改变原来fd_set的值，需要copy一份
        fd_set tmp = readfdset;
        // select 返回ready的fd数
        // 参数有:读fd_set，写fd_set,异常fd_set,timeout
        int fds = select(maxfd + 1, &tmp, NULL, NULL, NULL);
        printf("select fds=%d\n", fds);

        // 失败返回-1
        if (fds < 0) {
            printf("select() fail\n");
            perror("select() fail");
            break;
        }
        // 超时,select的最后一个值设置(结构体)
        if (fds == 0) {
            printf("select() timeout\n");
            continue;
        }

        // 检查所有的fd
        // select 采用bitmap去存储fd集合,默认1024个
        // for循环遍历所有可能有"事件"的fd
        for (int eventfd = 0; eventfd <= maxfd; ++eventfd) {
            // 非法fd
            if (FD_ISSET(eventfd, &tmp) <= 0) {
                continue;
            }
            // listen的sock
            // 就是说有新的客户端连接上来了
            // 走accept流程,并且把clientfd 设置进去fd_set中
            // ps:select只是告诉你有事件来了,具体的操作还是要自己撸
            if (eventfd == listensock) {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(listensock, (sockaddr *) &client, &len);
                if (clientsock < 0) {
                    printf("accept fail\n");
                    continue;
                }
                printf("client(%s %d) socket=%d connect ok\n",
                       inet_ntoa(client.sin_addr), client.sin_port, clientsock);

                // 把进的clientsock加入集合
                // 注意别加入到tmp中
                FD_SET(clientsock, &readfdset);
                if (maxfd < clientsock) {
                    maxfd = clientsock;
                }
                continue;
            } else {
                // clientfd的读|断开
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));

                ssize_t isize = read(eventfd, buffer, sizeof(buffer));
                // 对方关闭socket或出错
                if (isize <= 0) {
                    printf("client(eventfd=%d) disconnected\n", eventfd);
                    close(eventfd);
                    // 移除fd_set
                    FD_CLR(eventfd, &readfdset);
                    if (eventfd == maxfd) {
                        for (int i = maxfd; i > 0; i--) {
                            if (FD_ISSET(i, &readfdset)) {
                                maxfd = i;// 重新设置maxfd
                                break;;
                            }
                        }
                    }
                    printf("maxfd = %d\n", maxfd);
                    continue;
                }
                printf("recv(eventfd=%d),size(%zd):%s\n",
                       eventfd, isize, buffer);
                // 原数据返回
                write(eventfd, buffer, sizeof(buffer));
            }

        }
    }
    return 0;
}

int initserver(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("socket() fail");
        return -1;
    }

    // linux避免重启服务时候短暂的端口占用
    int opt = 1;
    unsigned int len = sizeof(opt);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, len);
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, len);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("bind() fail\n");
        return -1;
    }

    // tcp连接队列5个
    if (listen(sock, 5) != 0) {
        printf("listen fail\n");
        close(sock);
        return -1;
    }
    return sock;
}