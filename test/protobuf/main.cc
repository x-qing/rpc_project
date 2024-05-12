// #include "test.pb.h"
#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int main(){

/*
    // 封装了login请求对象的数据
    LoginRequest req;

    req.set_name("zhang san");
    req.set_pwd("123456");

    // 序列化
    std::string send_str;
    if(req.SerializeToString(&send_str)){
        std::cout << send_str.c_str() << std::endl;
    }


    // 反序列化 从send_str反序列化
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str)){
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl; 
    }
*/
    // LoginResponse rsp;
    // ResultCode *rc = rsp.mutable_result();   // 调用mutable方法，内容可改变
    // rc->set_errcode(1);
    // rc->set_errmsg("登录处理失败了");

    GetFriendListResponse rsp;
    ResultCode *rc = rsp.mutable_result();  // 获取对象的指针
    rc->set_errcode(0);

    // 添加列表
    User *user1 = rsp.add_friend_list();
    user1->set_age(20);
    user1->set_name("zhang san");
    user1->set_sex(User::MAN);

    User *user2 = rsp.add_friend_list();
    user2->set_age(22);
    user2->set_name("li si");
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl;
    return 0;
}