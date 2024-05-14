#pragma once
// Mprpc框架提供的日志系统

#include "lockqueue.h"

enum LogLevel{
    INFO,   // 普通日志信息
    ERROR,  // 错误信息
};

class Logger{
public:

    // 获取用户单例
    static Logger& GetInstance();

    // 设置日志级别
    void setLogLevel(LogLevel level);
    
    // 写日志
    void Log(std::string msg);
private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    int m_loglevel;
    LockQueue<std::string> m_lckQue;   // 日志缓冲队列
};


// Log_INFO("This is a log message: %s", "Hello, world!");
// 在这个例子中，logmsgformat 就是字符串 "This is a log message: %s"，
// ##__VA_ARGS__ 部分会将可变参数 "Hello, world!" 传递给 snprintf 函数，最终生成完整的日志消息。

#define Log_INFO(logmsgformat,...) \
    do \
    { \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c); \
    }while(0);


#define Log_ERR(logmsgformat,...) \
    do \
    { \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c); \
    }while(0);