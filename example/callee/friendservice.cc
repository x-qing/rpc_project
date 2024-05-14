#include <iostream>
#include <string>
// #include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "friend.pb.h"
#include <vector>

#include "logger.h"

class FriendService : public fixbug::FriendServiceRpc{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid){
        std::cout << "do GetFriendsList service! userid :" << userid << std::endl; 
        std::vector<std::string> vec;
        vec.push_back("li si");
        vec.push_back("xiao ming");
        return vec;
    }

    // 框架来进行调用
    // 重写基类的GetFriendsList方法
    virtual void GetFriendsList(::google::protobuf::RpcController* controller,
                    const ::fixbug::GetFriendsListRequest* request,
                    ::fixbug::GetFriendsListResponse* response,
                    ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();
        std::vector<std::string> friendList = GetFriendsList(userid);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        // response->add_friends
        for(std::string &name : friendList){
            // 返回的是一个字符串的指针，
            std::string *p = response->add_friends();
            *p = name;
        }

        done->Run();
    }
};


int main(int argc,char **argv){

    // Log_INFO("first log message!");
    // Log_ERR("%s:%s:%d!",__FILE__,__FUNCTION__,__LINE__);
    
    // 调用框架的初始化操作
    // 启动时需要读取配置文件，需要知道ip地址和端口号，
    // provider -i config.conf  读取config.conf里面的配置文件
    MprpcApplication::Init(argc,argv);


    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    // 数据的序列化、反序列化网络的数据收发
    RpcProvider provider;   // 发布服务  使用moduo库
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点，run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

}