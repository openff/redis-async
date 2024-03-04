#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <hiredis/async.h>
#include <hiredis/hiredis.h>
#include <hiredis/adapters/libev.h>
/*
g++ -std=c++11 -o async-redis async-redis.cpp -g -L/usr/local/lib -lhiredis -lev
sync 10w->25s   async-redis:10w ->  1.4s


test write cmd count:100000
count:100000  steady_clock time:1477.4
system time: 1477.55
*/
using namespace std;
static int g_iCmdCallback = 0;
chrono::steady_clock::time_point start;
chrono::system_clock::time_point systemStart;
void sendcmd(redisAsyncContext *pRdsContext, void *pReply, void *pData)
{
    int *count = (int *)pRdsContext->data;
    //printf("redis cmd call back, err code = %d, err msg = %s!\n", pRdsContext->err, pRdsContext->errstr);
    // 处理回调函数逻辑
    if (nullptr == pReply)
    {
        cout << "callback repplay nullptr\n";
        return;
    }
    redisReply *prdsReply = (redisReply *)pReply;
    if (REDIS_REPLY_NIL == prdsReply->type)
    {
        cout << "reply nil\n";
        return;
    }

    // 消息处理
    // end

    if (++g_iCmdCallback >= *count)
    {
        /*exit*/
        auto end = chrono::steady_clock::now();
        chrono::duration<double, std::milli> sum = static_cast<chrono::duration<double, std::milli>>(end - start);
        cout << "count:" << *count << "  steady_clock time:" << sum.count() << "\n";

        auto systemEnd = std::chrono::system_clock::now();
        chrono::duration<double, std::milli>  duration = (systemEnd - systemStart);
        std::cout << "system time: " << duration.count() << "\n";

        redisAsyncDisconnect((redisAsyncContext *)pRdsContext); 
    }
}
// 打包命令发送
void SendSetRequest(const redisAsyncContext *rdsConText)
{
    start = chrono::steady_clock::now();
    systemStart = chrono::system_clock::now();

    int *count = (int *)rdsConText->data;
    for (int i = 0; i < *count; i++)
    {
        const char *ppArg[] = {
            "SET",
            nullptr,
            nullptr};

        char szKey[64];
        snprintf(szKey, sizeof(szKey), "key-%d", i);
        ppArg[1] = szKey;

        char szValue[1024];
        char szTestInfo[64];
        memset(szTestInfo, 'a', sizeof(szTestInfo));
        snprintf(szValue, sizeof(szValue), "value-%d-%s", i, szTestInfo);
        ppArg[2] = szValue;

        size_t szArglen[] = {
            strlen("SET"),
            strlen(szKey),
            strlen(szValue)};

        //cout << "iArgSize: " << sizeof(ppArg) / sizeof(ppArg[0]) << ", ppArg: " << ppArg << ", szArglen[1]: " << szArglen[1] << "\n";

        redisAsyncCommandArgv((redisAsyncContext *)rdsConText, sendcmd, nullptr, sizeof(ppArg) / sizeof(ppArg[0]), ppArg, szArglen);
    }
}
void connectFun(const struct redisAsyncContext *redisConText, int status)
{

    std::cout << "connect callback,status=" << status << "\n";
    int *count = static_cast<int *>(redisConText->data);
    std::cout << "test write cmd count:" << *count << "\n";

    SendSetRequest(redisConText);
}
void DesconnectFun(const struct redisAsyncContext *redisConText, int status)
{
    cout << "disconnect callback ,status" << status << "\n";
}
int main(int argc, const char *argv[])
{

    if (argc >= 3)
    {
        const char *ip = (argv[1]);
        int port = std::stoi(argv[2]);
        int count = std::stoi(argv[3]);

        redisAsyncContext *redisConText = redisAsyncConnect(ip, port);
        if (redisConText->err != REDIS_OK)
        {
            std::cout << "async redis connect failed\n";
            return 1;
        }
        // 外带数据
        redisConText->data = &count;
        struct ev_loop *LOOP = EV_DEFAULT;
        // redis 与 libev绑定
        redisLibevAttach(LOOP, redisConText);

        // 连接回调
        redisAsyncSetConnectCallback(redisConText, connectFun);
        // 断开回调
        redisAsyncSetDisconnectCallback(redisConText, DesconnectFun);

        ev_run(LOOP, 0);
    }
    else
    {
        std::cout << "请输入redis ip port count" << std::endl;
    }

    return 0;
}