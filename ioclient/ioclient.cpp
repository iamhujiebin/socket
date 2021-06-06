#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Using ./ioclient addr port\n");
        return -1;
    }
    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() fail\n");
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
        close(sockfd);
        return -1;
    }
    printf("connect ok\n");
    // 读写数据
    for (int i = 0; i < 10; ++i) {
        memset(buf, 0, sizeof(buf));
        printf("input:");
        scanf("%s", buf);
        if (write(sockfd, buf, sizeof(buf)) <= 0) {
            printf("write fail\n");
            return -1;
        }
        memset(buf, 0, sizeof(buf));
        if (read(sockfd, buf, sizeof(buf)) <= 0) {
            printf("read fail\n");
            return -1;
        }
        printf("read back %s\n", buf);
    }
    return 0;
}
