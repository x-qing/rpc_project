#pragma once

// zookeeper是基于c的，这里使用c++做一下封装
#include <semaphore.h>
#include <string>
#include <zookeeper/zookeeper.h>

class ZkClient{
public:
    ZkClient();
    ~ZkClient();

    // zookeeper启动连接zookeeper server
    void Start();

    // 在zookeeper上根据指定的节点创建znode节点
    // 默认是永久性节点
    void Create(const char *path,const char *data,int datalen,int state=0);

    // 根据参数指定的znode节点路径来获取znode的值
    std::string GetData(const char *path);

private:
    // zk客户端的句柄
    zhandle_t *m_zhandle;
};
