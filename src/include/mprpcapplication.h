#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// rpc框架的初始化类
// 使用单例模式
class MprpcApplication{
public:
    // 初始化一次就行
    static void Init(int argc,char **argv);
    static MprpcApplication& getInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;

    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};

