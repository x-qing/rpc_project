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

    // stub.Login();   // RpcChannel->RpcChannel::callMethod方法，集中来做所有rpc方法调用的参数序列化和网络发送



    return 0;
}
