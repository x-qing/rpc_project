// rpc方法的提供者

#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetfriendLists
*/

// using namespace fixbug;   // 要使用命名空间

class UserService : public fixbug::UserServiceRpc{  // 使用在rpc服务的发布端
    bool Login(std::string name,std::string pwd){
        std::cout << "loacl service name : "<< name << std::endl;
        std::cout << "loacl service pwd : "<< pwd << std::endl;
        return false;
    }

    bool Register(uint32_t id,std::string name,std::string pwd){
        std::cout << "doing local service:Register" << std::endl;
        std::cout << "id:"<<id << " name:"<<name << " pwd:"<<pwd<<std::endl;
        return true;
    }


    // 远端发过来的rpc请求，是先被框架给接收，然后把请求进行上报
    // 得到request请求，做本地业务处理，处理完后交给response，再由rpc发送过去
    // 重写基类UserServiceRpc的虚函数Login方法，下面这些方法是框架直接调用的
    /*
        1. caller  ===>  Login(LoginRequest)  ===> muduo ===> callee
        2. callee  ===>  远端请求Login(LoginRequest) ===>交到下面重写的login方法。
    */
    void Login( ::google::protobuf::RpcController* controller,
                const ::fixbug::LoginRequest* request,
                ::fixbug::LoginResponse* response,
                ::google::protobuf::Closure* done)
    {
        // 框架给业务上报请求参数，业务获取相应数据，做本地业务
        std::string name  = request->name();
        std::string pwd = request->pwd();
        
        // 本地业务
        bool login_result = Login(name,pwd); 

        // 把响应写入，以及错误码和错误消息
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(1);
        code->set_errmsg("Login do error!");
        response->set_success(login_result);

        // 调用执行回调操作   执行响应数据的序列化和网络发送（由框架完成）
        done->Run();
    }

    // 注册用的远程服务
    virtual void Register(::google::protobuf::RpcController* controller,
                    const ::fixbug::RegisterRequest* request,
                    ::fixbug::RegisterResponse* response,
                    ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id,name,pwd);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);

        done->Run();
    }
    

};




// class UserService{
// public:
//     // 用户对象的登录密码
//     bool Login(std::string name,std::string pwd){
//         std::cout << "loacl service name : "<< name << std::endl;
//         std::cout << "loacl service pwd : "<< pwd << std::endl;
//     }
// };


/*
int main(){
    UserService us;
    // 同一个进程调用Login方法可以直接调用，通过对象名就可以了
    // 但是对于不同进程怎么调用呢？
    // 同一机器上的其他进程
    // 不同机器上的其他进程
    us.Login("aaa","123");
    return 0;
}
*/

int main(int argc,char **argv){

    // 调用框架的初始化操作
    // 启动时需要读取配置文件，需要知道ip地址和端口号，
    // provider -i config.conf  读取config.conf里面的配置文件
    MprpcApplication::Init(argc,argv);


    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    // 数据的序列化、反序列化网络的数据收发
    RpcProvider provider;   // 发布服务  使用moduo库
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点，run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

}