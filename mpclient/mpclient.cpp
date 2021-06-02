#include <iostream>
#include "FreeCPlus/_freecplus.h"

CTcpClient TcpClient;

bool biz000(); // 心跳
bool biz001(); // 登录
bool biz002(); // 余额

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Using:./mpclient ip port\n");
        return -1;
    }
    if (!TcpClient.ConnectToServer(argv[1], atoi(argv[2]))) {
        printf("connect fail :%s %d\n", argv[1], atoi(argv[2]));
        return -1;
    }
    if (!biz000()) {
        return -1;
    }

    sleep(1);

    biz001(); // 登录

    sleep(1);

    biz002(); // 余额查询
}

bool biz000() {
    char strbuffer[1024];
    memset(strbuffer, 0, sizeof(strbuffer));
    snprintf(strbuffer, 1000, "<bizcode>0</bizcode>");
    printf("发送:%s\n", strbuffer);
    if (!TcpClient.Write(strbuffer)) {
        return false;
    }
    if (!TcpClient.Read(strbuffer, 20)) {
        return false;
    }
    printf("接收:%s\n", strbuffer);
    return true;
}

bool biz001() {
    char strbuffer[1024];
    memset(strbuffer, 0, sizeof(strbuffer));
    snprintf(strbuffer, 1000, "<bizcode>1</bizcode><username>jiebin</username><password>123456</password>");
    printf("发送:%s\n", strbuffer);

    if (!TcpClient.Write(strbuffer)) {
        printf("发送失败");
        return false;
    }
    memset(strbuffer, 0, sizeof(strbuffer));
    if (!TcpClient.Read(strbuffer, 30)) {
        printf("接收失败");
        return false;
    }
    int iretcode = -1;
    GetXMLBuffer(strbuffer, "retcode", &iretcode);
    if (iretcode) {
        printf("身份验证失败");
        return false;
    }
    printf("身份验证成功\n");
    return true;
}

bool biz002() {
    char strbuffer[1024];
    memset(strbuffer, 0, sizeof(strbuffer));
    snprintf(strbuffer, 1000, "<bizcode>2</bizcode><cardid>62620000000001</cardid>");
    if (!TcpClient.Write(strbuffer)) {
        printf("发送失败\n");
        return false;
    }
    memset(strbuffer, 0, sizeof(strbuffer));
    if (!TcpClient.Read(strbuffer, 30)) {
        printf("接收失败\n");
        return false;
    }
    int iretcode = -1;
    char *ye;
    char *message;
    GetXMLBuffer(strbuffer, "retcode", &iretcode);
    GetXMLBuffer(strbuffer, "ye", ye);
    GetXMLBuffer(strbuffer, "message", message);
    if (iretcode) {
        printf("获取余额失败:%s", message);
        return false;
    }
    printf("获取余额成功:¥%s", ye);
    return true;
}