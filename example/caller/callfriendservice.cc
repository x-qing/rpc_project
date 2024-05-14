// rpc方法的消费者
// 业务代码

#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"
// #include "mprpcchannel.h"
// #include "mprpccontroller.h"


int main(int argc,char** argv){
    // 整个程序启动以后，使用mprpc框架来享受rpc服务调用，一定要先调用框架的初始化函数（只用一次初始化操作）
    MprpcApplication::Init(argc,argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    // rpc方法的请求参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    // 响应是返回的
    // rpc方法的响应参数
    fixbug::GetFriendsListResponse response;

    // 调用完成后同步等待等待rpc的响应结果，然后直接访问response即可
    /*
        上述条件是：整个rpc请求一定是成功的。rpc请求已经发到了远端，而且远端已经进行了回复
        这里都是通过调用sub的CallMethod方法来实现的，但是如果CallMethod里面直接在序列化就失败的话，那返回的结果肯定是有问题的

        ::PROTOBUF_NAMESPACE_ID::RpcController* controller 可以存储一些控制信息，表示当前的调用是个什么样的状态

        // contents of the response message are undefined.
        virtual bool Failed() const = 0;

        // If Failed() is true, returns a human-readable description of the error.
        virtual std::string ErrorText() const = 0;

    */
    // 记录调用过程中出现的一些错误信息
    MprpcController controller;
    stub.GetFriendsList(&controller,&request,&response,nullptr);

    // rpc调用完成，读取调用的结果
    if(controller.Failed()){
        // rpc调用过程中出现了错误
        std::cout << controller.ErrorText() << std::endl;
    }else{
        if(response.result().errcode() == 0){
            std::cout << "rpc getfriendslist response success!" << std::endl;
            // std::vector<std::string> 
            int size = response.friends_size();
            for(int i =0;i<size;++i){
                std::cout << "index:" << (i+1) << "  name:" << response.friends(i) << std::endl;
            }
        }else{
            std::cout << "rpc getfriendslist response error :" << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}
