#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// socket服务端
// 第一步:创建服务器的socket
// 第二步:bind地址和端口
// 第三步:把socket设置为listen状态
// 第四步:接收客户端的连接
// 第五步:与客户端通讯
// 第六步:关闭socket,释放资源
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Using ./server port \n Example ./server 5005");
        return -1;
    }
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket fail");
        return -1;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;// 协议族，在socket编程中只能是AF_INET。
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);// 任意ip地址,网络字节顺序
    servaddr.sin_port = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        perror("bind err");
        return -1;
    }
    if (listen(listenfd, 5) != 0) {
        perror("listen err");
        return -1;
    }
    printf("start to listen:%s\n", argv[1]);

    int clientfd;
    int socklen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    clientfd = accept(listenfd, (struct sockaddr *) &clientaddr, (socklen_t *) &socklen);
    printf("客户端(%s) 已连接\n", inet_ntoa(clientaddr.sin_addr));

    char buffer[1024];
    while (true) {
        int iret;
        memset(buffer, 0, sizeof(buffer));
        if ((iret = recv(clientfd, buffer, sizeof(buffer), 0)) <= 0) {
            printf("iret=%d\n", iret);
            break;
        }
        printf("接收:%s\n", buffer);
        strcpy(buffer, "ok");

        if ((iret = send(clientfd, buffer, strlen(buffer), 0)) <= 0) {
            printf("send fail:%d\n", iret);
            break;
        }
        printf("发送:%s\n", buffer);
    }
    close(listenfd);
    close(clientfd);
    return 0;
}
