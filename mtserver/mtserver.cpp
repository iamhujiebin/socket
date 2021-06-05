#include "FreeCPlus/_freecplus.h"
#include "mtserver.h"
#include "pthread.h"

vector<pthread_t> vphid; // 线程id

CLogFile logFile;
CTcpServer tcpServer;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Using:./mtserver_biz port logfile\nExample:./mtserver_biz 5005 /tmp/mtserver_biz.log\n\n");
        return -1;
    }

    // 关闭所有的信号
    for (int i = 0; i < 100; ++i) {
        signal(i, SIG_IGN);
    }
    // 打开日志文件
    if (!logFile.Open(argv[2], "a+")) {
        printf("logfile.Open(%s) failed.\n", argv[2]);
        return -1;
    }
    // 设置 2 15 信号函数
    signal(SIGINT, mainexit);
    signal(SIGTERM, mainexit);

    if (!tcpServer.InitServer(atoi(argv[1]))) {
        logFile.Write("TcpServer.InitServer(%s) failed.\n", argv[1]);
        return -1;
    }
    logFile.Write("Listen %d", atoi(argv[1]));

    while (true) {
        if (!tcpServer.Accept()) {
            logFile.Write("TcpServer.Accept() failed.\n");
            continue;
        }

        logFile.Write("客户端(%s)已经连接\n", tcpServer.GetIP());

        pthread_t pthid;
        if (pthread_create(&pthid, NULL, pthmain, (void *) (long) tcpServer.m_connfd) != 0) {
            logFile.Write("pthread_create failed.\n");
            return -1;
        }
        vphid.push_back(pthid);
    }
    return 0;
}

void *pthmain(void *arg) {
    pthread_cleanup_push(pthmainexit, arg);
        pthread_detach(pthread_self()); // 分离线程,主线程的子线程不能一直是joinable
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // 设置取消方式为立即取消,还有一种方式是取消点取消(一些系统调用)
        int sockfd = (int) (long) arg; // 强转,指针=long=八字节

        int ibuflen = 0;
        char strrecvbuffer[1024], strsendbuffer[1024];
        while (true) {
            memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
            memset(strsendbuffer, 0, sizeof(strsendbuffer));
            if (!TcpRead(sockfd, strrecvbuffer, &ibuflen, 30)) {
                logFile.Write("已断开\n");
                break;
            }
            logFile.Write("接收：%s\n", strrecvbuffer);
            if (!_main(strrecvbuffer, strsendbuffer)) {
                logFile.Write("处理失败");
                break;
            }
            logFile.Write("发送：%s\n", strsendbuffer);
            if (!TcpWrite(sockfd, strsendbuffer)) {
                logFile.Write("发送失败\n", strrecvbuffer);
                break;
            }
        }
    pthread_cleanup_pop(1);
    pthread_exit(0);
}

void mainexit(int sig) {
    logFile.Write("mainexit begin\n");

    // 关闭服务端socket
    tcpServer.CloseListen();

    // 取消全部线程
    for (int i = 0; i < vphid.size(); ++i) {
        logFile.Write("Cancel %ld\n", vphid[i]);
        pthread_cancel(vphid[i]);
    }
    logFile.Write("mainexit end\n");
    exit(0);
}

void pthmainexit(void *arg) {
    logFile.Write("pthmainexit being\n");

    // 关闭cilentfd socket

    close((int) (long) arg);

    // 从容器中删除本线程的id
    for (int i = 0; i < vphid.size(); ++i) {
        if (vphid[i] == pthread_self()) {
            logFile.Write("should remove:%d\n", pthread_self());
            vphid.erase(vphid.begin() + i);
        }
    }
    logFile.Write("pthmainexit end.\n");
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

