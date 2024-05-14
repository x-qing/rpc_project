#include "logger.h"
#include <time.h>
#include <iostream>


// 获取用户单例
Logger& Logger::GetInstance(){
    static Logger logger;
    return logger;
}


// 设置日志级别
void Logger::setLogLevel(LogLevel level){
    m_loglevel = level;
}


Logger::Logger(){
    // 启动专门的写日志线程
    std::thread writeLogTask([&](){
        for(;;){
            // 获取当天的日志信息，然后获取日志信息，写入相应的日志文件当中  a+
            time_t now = time(nullptr); // 获取的是秒数
            tm *nowtm = localtime(&now);
            char file_name [128];
            // 定义文件名
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);

            /*
                每一次信息的写入都伴随一次文件的打开和关闭
            */
            FILE *pf = fopen(file_name,"a+");

            if(pf == nullptr){
                std::cout << "logger file :" << file_name << "open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            // 
            std::string msg = m_lckQue.Pop();
            

            char time_buf[128]={0};
            sprintf(time_buf,"%d:%d:%d => [%s] => ",nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,(m_loglevel==INFO?"info":"error"));
            msg.insert(0,time_buf);
            msg.append("\n");

            fputs(msg.c_str(),pf);

            fclose(pf);
        }
    });
    
    // 设置分离线程 
    writeLogTask.detach();
}


// 写日志
void Logger::Log(std::string msg){
    // 把日志信息写入lockqueue缓冲区，，外部使用，
    // 外部线程直接调用Log函数，把数据写入lockqueue缓冲区就行，其余的不用管。
    m_lckQue.Push(msg);
}

