#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"



// 用户怎么像rpcprovide注册它支持的rpc服务？怎么存储rpc服务？
// 怎么通过接收到远程的rpc请求来找到相应的服务？
// 框架不可能依赖于某个业务的具体的类，这里统一用的指针来实现

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

    std::cout << "RpcProvider start service at ip:" << ip << "  port:"<<port << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop();//启动了epoll_wait()
}

// 请求连接---短连接
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn){
    if(!conn->connected()){
        // 断开连接了
        conn->shutdown();
    }
}


/*
    在框架内部，RpcProvider和RpcConsumer协商好之间通信的protobuf数据类型
    服务对象：service_name method_name args，定义proto的message类型进行数据头的序列化和反序列化
                                            args_size。
    通过service_name找到service对应的信息（service对象以及里面的方法），通过method_name找到method方法对象

    header_size + header_str + args_str (区分服务名和方法)，tcp的粘包问题。
    10 -- "10"
    100 -- "100"
    header_size按照字节的方式
    std::string insert和copy方法

*/

// 已建立连接用户的读写事件回调，如果远程有一个rpc服务的调用请求，那么Onmessage方法就会相应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer* buffer,muduo::Timestamp){
    /*
    接收到的是字节流
    */
    // 这里是网络上接受的远程rpc调用请求的字符流 Login  args
    std::string recv_buf = buffer->retrieveAllAsString();
    
    // 从字符流中读取前四个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size,4,0);

    // 根据header_size读取数据头的原始字符流,反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        // 反序列化成功
        // 获取对应的值
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }else{
        // 反序列化失败
        std::cout << "rpc_header_str: "<<rpc_header_str << "parse error!" << std::endl;
        return;
    }

    // 获取参数字符流数据
    std::string args_str = recv_buf.substr(4+header_size,4+header_size+args_size);

    // 打印信息
    std::cout << "-----------------provider---------------" << std::endl;
    std::cout << "head_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "--------------------------------------" << std::endl;
    
    // 获取了远程rpc框架函数调用的对象名，函数名和参数

    // 获取service对象和method对象
    auto it = m_serviceInfoMap.find(service_name);
    if(it == m_serviceInfoMap.end()){
        std::cout << service_name << "is not exits!" << std::endl;
    }
    // 获取方法
    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end()){
        std::cout << service_name << ":" << method_name << " is not exits" << std::endl;
    }
    google::protobuf::Service *service = it->second.m_service;   // new userService
    const google::protobuf::MethodDescriptor* method = mit->second;   // Login

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    // 请求方法的参数是从远端传送来的
    if(!request->ParseFromString(args_str)){
        std::cout << "reques parse error! content:"<< args_str << std::endl;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();


    // 给下面的method方法的调用，绑定一个Closure的回调函数

    // See Closure.
    // template <typename Class, typename Arg1, typename Arg2>
    // inline Closure* NewCallback(Class* object, void (Class::*method)(Arg1, Arg2),
    //                             Arg1 arg1, Arg2 arg2) {
    //   return new internal::MethodClosure2<Class, Arg1, Arg2>(
    //     object, method, true, arg1, arg2);
    // }

    // 类型推导出错了，直接指定类型
    google::protobuf::Closure* done =  
        google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>(this,&RpcProvider::SendRpcResponse,conn,response);

    // 在框架上根据远端rpc请求，调用对应的方法
    // new UserService().Login(constroller,request,response,done)
    service->CallMethod(method,nullptr,request,response,done);

    // done.run()// 执行的就是绑定好的回调函数

}


void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* response){
    std::string response_str;
    if(response->SerializeToString(&response_str)){
        // 序列化成功后，通过网络把rpc方法执行的结果发送给rpc方法的调用方
        conn->send(response_str);
    }else{
        std::cout << "serialize response_str error!" << std::endl; 
    }
    conn->shutdown();   // 模拟http的短链接服务，由rpcprovider主动断开连接
}