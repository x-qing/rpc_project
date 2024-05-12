#pragma once

// 框架提供的专门用来发布rpc服务的网络对象类
#include "google/protobuf/service.h"

class RpcProvider{
public:
    // 定义一个抽象类指针，来接收任意类型的方法
    // 框架提供的外部使用的，可以发布rpc方法的函数接口，必须站在抽象的角度来看待这个问题
    // 这样就可以接收任意类型的rpc服务对象
    void NotifyService(google::protobuf::Service* service);

    // 启动rpc服务节点，开始提供rpc远程调用服务
    void Run();
};