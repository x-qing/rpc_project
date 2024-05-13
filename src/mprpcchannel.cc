#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include <string>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <error.h>
#include "mprpcapplication.h"
// #include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <netinet/in.h>



// 调用方的实现
// rpc方法调用的数据序列化和网络发送
/*
header_size + service_name method_name args_size + args
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                        google::protobuf::Message* response, google::protobuf::Closure* done)
{
    //   const ServiceDescriptor* service() const;
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();   // 得到service_name
    std::string method_name = method->name();  // 得到method_name

    // 获取参数的序列化字符串长度，args_size
    std::string args_str;
    // 序列化操作
    uint32_t args_size = 0;
    if(request->SerializeToString(&args_str)){
        args_size = args_str.size();
    }else{
        std::cout << "serialize request error : " << std::endl;
        return;
    }

    // 定义rpc的请求header
    mprpc::RpcHeader rpcheader;
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);
    rpcheader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcheader.SerializeToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }else{
        std::cout << "serialize rpc header error!" << std::endl;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    // 写四个字节
    send_rpc_str.insert(0,std::string((char*)&header_size,4));
    send_rpc_str += rpc_header_str;  // rpch
    send_rpc_str += args_str;   // args

        // 打印信息
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "head_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "--------------------------------------" << std::endl;

    // 使用tcp编程完成rpc方法的远程调用
    int clientfd = socket(AF_INET,SOCK_STREAM,0);

    if(-1 == clientfd){
        std::cout << "create socket error! error:" << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    std::string ip = MprpcApplication::getInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getInstance().GetConfig().Load("rpcserverport").c_str());
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if(-1 == connect(clientfd,(struct sockaddr*)&server_addr,sizeof(server_addr))){
        std::cout << "connect error! error:" << errno << std::endl;
        close(clientfd);
        exit(EXIT_FAILURE);
    }

    // 发送rpc请求
    if(-1 == send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0)){
        std::cout << "send error! error:" << errno << std::endl;
        // exit(EXIT_FAILURE);
        close(clientfd);
        return;
    }

    // 接受rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size;
    if(-1 == (recv_size = recv(clientfd,recv_buf,1024,0))){
        std::cout << "recv error! error:" << errno << std::endl;
        // exit(EXIT_FAILURE);
        close(clientfd);
        return;
    }

    // 反序列化
    // 准备用ParseFromString来进行反序列化，但是出错了
    // 在使用string的构造函数进行初始化的时候，遇到\0就直接终止了，导致反序列化的结果失败   
    // std::string response_str(recv_buf,0,recv_size);   // 这里出现问题
    if(!response->ParseFromArray(recv_buf,recv_size)){
        std::cout << "parse error! response_str:" << recv_buf << std::endl;
        close(clientfd);
        return;
    } 
    close(clientfd);
}