#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// socket 客户端
// 第一步:创建客户端socket
// 第二步:向服务器发起连接请求
// 第三步:与服务器通讯,发报文
// 第四步:关闭socket,释放资源
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Using:./client ip port\n");
    }
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket fail");
        return -1;
    }
    struct hostent *h;
    if ((h = gethostbyname(argv[1])) == 0) { // 指定服务端的ip地址
        printf("gethostbyname fail");
        return -1;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        perror("connect fail");
        return -1;
    }

    char buffer[1024];
    for (int i = 0; i < 10; ++i) {
        int iret;
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "报文%d", i);

        if ((iret = send(sockfd, buffer, strlen(buffer), 0)) <= 0) {
            printf("send fail:%d\n", iret);
            break;
        }
        printf("发送：%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));

        if ((iret = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0) {
            printf("recv fail%d\n", iret);
            break;
        }
        printf("接收:%s\n", buffer);
    }
    close(sockfd);
}
