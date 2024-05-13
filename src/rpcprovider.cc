#include "rpcprovider.h"
#include "mprpcapplication.h"
#include <functional>

// 用户怎么像rpcprovide注册它支持的rpc服务？怎么存储rpc服务？
// 怎么通过接收到远程的rpc请求来找到相应的服务？
void RpcProvider::NotifyService(google::protobuf::Service* service){

}

// 启动rpc服务节点，开始提供rpc远程调用服务
void RpcProvider::Run(){
    // 组合了TcpServer
    // std::unique_ptr<muduo::net::TcpServer> m_tcpserverPtr;

    // InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
    // 获取ip和端口
    std::string ip = MprpcApplication::getInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);
    /*  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg,
            Option option = kNoReusePort);
    */
    // 创建tcpserver对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");

    // 绑定连接回调和消息读写方法。
    // 分离了网络代码和业务代码
    // 连接回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,
                                                            std::placeholders::_2,std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    std::cout << "RpcProvider start service at ip" << ip << "port" << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop();//启动了epoll_wait()
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn){

}

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp){

}