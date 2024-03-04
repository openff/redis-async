#include<iostream>
#include <string>
#include <chrono>
#include <ctime>
#include<cstring>
#include<hiredis/hiredis.h>

/**
 * 
10w 25s
count:100000  steady_clock time:25479.6
system time: 25479.8

*/

using namespace std;
static int g_iCmdCallback = 0;
chrono::steady_clock::time_point start;
chrono::system_clock::time_point systemStart;
int count;
void SendSetRequest( redisContext *rdsConText)
{
    start = chrono::steady_clock::now();
    systemStart = chrono::system_clock::now();

    
    for (int i = 0; i < count; i++) {
        const char *ppArg[] = {
            "SET",
            nullptr,
            nullptr
        };

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
            strlen(szValue)
        };

        redisReply* reply = (redisReply*)redisCommandArgv(rdsConText, sizeof(ppArg)/sizeof(ppArg[0]), ppArg, szArglen);
        if (reply == nullptr) {
            std::cerr << "Failed to execute Redis command" << std::endl;
            return;
        }
         // 处理命令结果
        if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
            //std::cout << "SET command executed successfully" << std::endl;
            if (++g_iCmdCallback >= count)
            {
                /*exit*/
                auto end = chrono::steady_clock::now();
                chrono::duration<double, std::milli> sum = static_cast<chrono::duration<double, std::milli>>(end - start);
                cout << "count:" << count << "  steady_clock time:" << sum.count() << "\n";

                auto systemEnd = std::chrono::system_clock::now();
                chrono::duration<double, std::milli>  duration = (systemEnd - systemStart);
                std::cout << "system time: " << duration.count() << "\n";
                break;
            }   
        } else {
            std::cerr << "Failed to execute SET command" << std::endl;
        }
        // 释放命令结果内存
         freeReplyObject(reply);
    }
}
int main(int argc, const char *argv[]){
   if (argc >= 3)
    {
        const char *ip = (argv[1]);
        int port = std::stoi(argv[2]);
        count = std::stoi(argv[3]);
        redisContext* rdsConText = redisConnect(ip,port);
        if(rdsConText == nullptr || rdsConText->err){
            std::cerr << "Failed to connect to Redis: " << rdsConText->errstr << std::endl;
            return 1;
        }
        SendSetRequest(rdsConText);
        
        // 关闭Redis连接
        redisFree(rdsConText);
    }
    else
    {
        std::cout << "请输入redis ip port count" << std::endl;
    }

    return 0;
}