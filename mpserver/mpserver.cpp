#include <iostream>
#include "FreeCPlus/_freecplus.h"

CLogFile logFile;
CTcpServer TcpServer;
int connCount = 0;

// 程序退出时调用的函数
void FatherEXIT(int sig); // 父进程退出函数
void ChildEXIT(int sig); // 子进程退出函数

// 业务函数
// 处理业务的主函数。
bool _main(const char *strrecvbuffer, char *strsendbuffer);

// 心跳报文。
bool biz000(const char *strrecvbuffer, char *strsendbuffer);

// 身份验证业务处理函数。
bool biz001(const char *strrecvbuffer, char *strsendbuffer);

// 查询余客业务处理函数。
bool biz002(const char *strrecvbuffer, char *strsendbuffer);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Using: ./mpserver port logfile.log");
        return -1;
    }
    printf("starting MPServer :%d, logfile:%s\n", atoi(argv[1]), argv[2]);
    // 忽略所有信号
    for (int ii = 0; ii < 100; ++ii) {
        signal(ii, SIG_IGN);
    }
    // 打开日志文件
    if (!logFile.Open(argv[2], "a+")) {
        printf("logfile.Open(%s) failed\n", argv[2]);
        return -1;
    }
    // 设置信号
    signal(SIGINT, FatherEXIT);
    signal(SIGTERM, ChildEXIT);

    // 初始化通讯端口
    if (!TcpServer.InitServer(atoi(argv[1]))) {
        logFile.Write("TcpServer init fail");
        FatherEXIT(-1);
    }

    while (true) {
        if (!TcpServer.Accept()) {
            logFile.Write("TcpServer Accept fail\n");
            continue;
        }
        // 有客户端来了!
        // 多进程fork,会有冗余的拷贝
        // 父进程可以关掉ClientFd
        // 子进程可以关掉ListenFd
        if (fork() > 0) {
            TcpServer.CloseClient();
            continue; // 继续等待accept
        }
        // 以下是子进程
        TcpServer.CloseListen();
        logFile.Write("客户端(%s) 已经连接\n", TcpServer.GetIP());

        char strrecvbuffer[1024], strsendbuffer[1024];

        while (true) {
            memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
            memset(strsendbuffer, 0, sizeof(strsendbuffer));

            // 30秒超时
            if (!TcpServer.Read(strrecvbuffer, 30)) {
                break;
            }
            logFile.Write("接收:%s\n", strrecvbuffer);
            if (!_main(strrecvbuffer, strsendbuffer)) {
                ChildEXIT(-1);
            }
            logFile.Write("发送:%s\n", strsendbuffer);
            if (!TcpServer.Write(strsendbuffer)) {
                break;
            }
        }
        logFile.Write("客户端(%s)已经断开\n", TcpServer.GetIP());
        ChildEXIT(-1);
    }
}

void FatherEXIT(int sig) {
    if (sig > 0) {
        signal(sig, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        logFile.Write("catching the signal(%d)\n", sig);
    }
    // 通知其它的子进程退出
    kill(0, 15);
    logFile.Write("父进程都退出了\n");
    TcpServer.CloseListen();
    exit(0);
}

void ChildEXIT(int sig) {
    if (sig > 0) {
        // SIG_IGN:忽略信号
        signal(sig, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
    }
    logFile.Write("子进程退出\n");
    TcpServer.CloseClient(); // 关掉子进程用到的clientFd
    exit(0);
}

bool _main(const char *strrecvbuffer, char *strsendbuffer)  // 处理业务的主函数。
{
    int ibizcode = -1;
    GetXMLBuffer(strrecvbuffer, "bizcode", &ibizcode);

    switch (ibizcode) {
        case 0:  // 心跳
            biz000(strrecvbuffer, strsendbuffer);
            break;
        case 1:  // 身份验证。
            biz001(strrecvbuffer, strsendbuffer);
            break;
        case 2:  // 查询余额。
            biz002(strrecvbuffer, strsendbuffer);
            break;

        default:
            logFile.Write("非法报文：%s\n", strrecvbuffer);
            return false;
    }

    return true;
}

// 身份验证业务处理函数。
bool biz001(const char *strrecvbuffer, char *strsendbuffer) {
    char username[51], password[51];
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    GetXMLBuffer(strrecvbuffer, "username", username, 50);
    GetXMLBuffer(strrecvbuffer, "password", password, 50);

    if ((strcmp(username, "jiebin") == 0) && (strcmp(password, "123456") == 0))
        sprintf(strsendbuffer, "<retcode>0</retcode><message>成功。</message>");
    else
        sprintf(strsendbuffer, "<retcode>-1</retcode><message>用户名或密码不正确。</message>");

    return true;
}

// 查询余额业务处理函数。
bool biz002(const char *strrecvbuffer, char *strsendbuffer) {
    char cardid[51];
    memset(cardid, 0, sizeof(cardid));

    GetXMLBuffer(strrecvbuffer, "cardid", cardid, 50);

    if (strcmp(cardid, "62620000000001") == 0)
        sprintf(strsendbuffer, "<retcode>0</retcode><message>成功。</message><ye>100.50</ye>");
    else
        sprintf(strsendbuffer, "<retcode>-1</retcode><message>卡号不存在。</message>");

    return true;
}

// 心跳报文
bool biz000(const char *strrecvbuffer, char *strsendbuffer) {
    sprintf(strsendbuffer, "<retcode>0</retcode><message>成功。</message>");
    return true;
}

