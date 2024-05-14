// rpc方法的消费者
// 业务代码

#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"


int main(int argc,char** argv){
    // 整个程序启动以后，使用mprpc框架来享受rpc服务调用，一定要先调用框架的初始化函数（只用一次初始化操作）
    MprpcApplication::Init(argc,argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // 响应是返回的
    // rpc方法的响应参数
    fixbug::LoginResponse response;

    /*
        void UserServiceRpc_Stub::Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                                    const ::fixbug::LoginRequest* request,
                                    ::fixbug::LoginResponse* response,
                                    ::google::protobuf::Closure* done) {
        channel_->CallMethod(descriptor()->method(0),
                            controller, request, response, done);
        }
    */
    // 里面调用的就是CallMethod方法
    // 发起rpc方法的调用，同步的rpc调用过程  MprpcChannel::callmethod()
    // RpcChannel->RpcChannel::callMethod方法，集中来做所有rpc方法调用的参数序列化和网络发送
    stub.Login(nullptr,&request,&response,nullptr);

    // rpc调用完成，读取调用的结果
    if(response.result().errcode() == 0){
        std::cout << "rpc login response success :" << response.success() << std::endl;
    }else{
        std::cout << "rpc login response error :" << response.result().errmsg() << std::endl;
    }

    // 演示远程调用发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("hhh");
    req.set_pwd("11111");

    fixbug::RegisterResponse rsp;
    
    // 以同步的方式发起rpc调用请求，等待返回结果
    stub.Register(nullptr,&req,&rsp,nullptr);
    if(rsp.result().errcode() == 0){
        std::cout << "rpc register response success :" << rsp.sucess() << std::endl;
    }else{
        std::cout << "rpc register response error :" << rsp.result().errmsg() << std::endl;
    }



    return 0;
}
