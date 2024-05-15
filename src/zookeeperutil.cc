#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

void global_watcher(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx){
    if(type == ZOO_SESSION_EVENT){   // 回调的消息类型是和会话相关的类型
        if(state == ZOO_CONNECTED_STATE){  // zkclient和zkserver连接成功
            sem_t *sem = (sem_t*)zoo_get_context(zh);  // 从指定句柄获取信号量
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient():m_zhandle(nullptr){

}

ZkClient::~ZkClient(){
    if(m_zhandle != nullptr){
        zookeeper_close(m_zhandle);  // 关闭句柄，释放资源
    }
}

// zookeeper启动连接zookeeper server
void ZkClient::Start(){
    std::string host = MprpcApplication::getInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::getInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /*
        该函数是一个异步操作，发送连接请求和接受请求的响应是一个异步的操作
        zookeeper_mt：多线程的版本
        zookeeper的API客户端程序提供了三个线程，
            API调用线程
            网络io线程，zookeeper_init底层创建（pthread_create  poll）了一个线程，专门用来发起网络的io
            watcher回调线程，
        m_zhandle成功表示创建句柄资源的成功，并不是连接server成功了

    */
    m_zhandle = zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
    if(nullptr == m_zhandle){
        std::cout << "Zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);   // 给执行句柄设置信号量

    // 初始化的信号量伟为0，会阻塞在这里，需要等待zkserver的响应才行
    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}   

// 在zookeeper上根据指定的节点创建znode节点
// 默认是永久性节点
void ZkClient::Create(const char *path,const char *data,int datalen,int state){
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    flag = zoo_exists(m_zhandle,path,0,nullptr);
    // 先判断path表示的znode节点是否存在，如果存在，就不再重复创建
    // 要创建的znode节点不存在
    if(ZNONODE == flag){
        flag = zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag == ZOK){
            std::cout << "znode create success... path:" << path << std::endl;
        }else{
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error... path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的znode节点路径来获取znode的值
std::string ZkClient::GetData(const char *path){
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);
    if(flag != ZOK){
        std::cout << "get znode error... path:" << path << std::endl;
        return "";
    }else{
        return buffer;
    }
}