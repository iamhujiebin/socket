void *pthmain(void *arg); // 线程主函数
void mainexit(int sig); // 主线程,信号2｜15的处理函数
void pthmainexit(void *arg); // 线程清理函数
bool _main(const char *strrecvbuffer, char *strsendbuffer);// 处理业务的主函数。
bool biz000(const char *strrecvbuffer, char *strsendbuffer);// 心跳报文。
bool biz001(const char *strrecvbuffer, char *strsendbuffer);// 身份验证业务处理函数。
bool biz002(const char *strrecvbuffer, char *strsendbuffer);// 查询余客业务处理函数。
