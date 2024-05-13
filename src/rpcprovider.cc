#include "rpcprovider.h"
#include "mprpcapplication.h"



// 用户怎么像rpcprovide注册它支持的rpc服务？怎么存储rpc服务？
// 怎么通过接收到远程的rpc请求来找到相应的服务？
// 框架不可能依赖于某个业务的具体的类，这里统一用基类的指针来实现

/*
    service_name ===>service描述 ===>
                                ===> service* 记录服务对象
                                method_name ==>method方法
*/
void RpcProvider::NotifyService(google::protobuf::Service* service){
    // 获取服务对象的描述信息，这里描述的就是服务的对象和方法（UserService-->Login()）

    ServiceInfo service_info;

    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    // 这里要包含对应的头文件
    std::string service_name = pserviceDesc->name();
    // 获取服务对象方法的数量，这里应该返回1，目前里面只有一个login方法
    int methodCnt = pserviceDesc->method_count();

    std::cout << "service_name:" << service_name << std::endl;


    for(int i = 0;i < methodCnt; ++i){
        //   const MethodDescriptor* method(int index) const;
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});
        std::cout << "method_name:" << method_name << std::endl;
    }
    service_info.m_service = service;
    m_serviceInfoMap.insert({service_name,service_info});

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