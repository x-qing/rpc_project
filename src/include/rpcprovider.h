#pragma once

// 框架提供的专门用来发布rpc服务的网络对象类
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Callbacks.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>


// 发布rpc的节点，这里可能会有很多的连接请求rpc的服务
// 这里使用高并发的muduo网络库来实现

class RpcProvider{
public:
    // 定义一个抽象类指针，来接收任意类型的方法
    // 框架提供的外部使用的，可以发布rpc方法的函数接口，必须站在抽象的角度来看待这个问题
    // 这样就可以接收任意类型的rpc服务对象
    void NotifyService(google::protobuf::Service* service);

    // 启动rpc服务节点，开始提供rpc远程调用服务
    void Run();

private:
    // // 组合了TcpServer
    // std::unique_ptr<muduo::net::TcpServer> m_tcpserverPtr;

    // 组合了Eventloop
    muduo::net::EventLoop m_eventLoop;

    // service服务类型信息
    struct ServiceInfo{
        google::protobuf::Service *m_service;   // 保存服务对象
        // 键值类型  保存服务方法
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string,ServiceInfo> m_serviceInfoMap;

    // 新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // 已建立连接的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
};