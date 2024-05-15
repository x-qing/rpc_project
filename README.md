#### 实现一个远程过程调用rpc框架，起具体调用流程如下：
    1.服务注册中心（知道要调用的服务具体在哪一个机器上）（zeekeeper）
    2.发起一个loacl call 远程调用函数
    3.把发起请求的函数以及参数以及远程调用的目标函数打包（protobuf）
    4.通过网络发送出去（muduo）
    5.收到网络包后，反序列化
    6.执行对应的调用函数，并返回结果
    7.再将结果序列化，然后发送出去
    8.调用大接收到后，反序列化得到具体的结果。

本项目的文件代码结构如下：

    bin：可执行文件
    build：项目编译文件
    lib：项目库文件
    src：源文件
    test：测试代码
    example：框架代码使用范例
    CMakeLists.txt: 顶层的cmake文件
    autobuild：项目一键编译脚本

相关知识点：

1.单机服务器存在的问题：模块的细微修改都要重新编译并部署整个项目；同时由于硬件资源的限制，容易出现性能瓶颈；同时由于每个模块对于配置要求的不集中（有些模块需要存储大量数据，依赖内存，磁盘。有些模块是cpu密集型的，更加依赖于cpu的性能、算力等）。
2.由此升级成集群的形式：但是集群虽然解决了单一服务器的性能瓶颈，但是在编译和部署以及配置要求不集中方面依旧没有得到解决，反而加大了对于资源的浪费以及部署的困难
3.继续升级成分布式的形式：对功能模块进行拆分，然后进行分布式的部署。对于业务繁忙的业务，就可以多部署几台服务器；而对于存储要求高的模块，可以给它部署在一台磁盘大但是cpu一般的服务器上...。这样就可以解决上述出现的问题，单一模块的修改这样只用重新编译该模块即可，不会影响其他的模块。

但是分布式的情况下，一个显而易见的问题就是，在服务器A上的某一模块上调用了某个函数，但是该函数的声明和实现却是在服务器B的某一模块中，如何解决上述问题？该项目主要就是基于protobuf提供的server服务以及muduo库提供的网络传输来实现rpc远程过程调用框架。

>以下为代码实现的详细步骤：

>安装配置protobuf
    
    首先下载相关protobuf版本的（本项目使用的是3.8.0）:
    下载地址：https://github.com/protocolbuffers/protobuf
    拷贝到虚拟机中（项目使用的是ubuntu16.04）
    解压：sudo tar -zxvf ***.tar.gz
    编译和安装： cd protobuf-3.19.4
                sudo ./autogen.sh
                sudo ./configure
                sudo make
                sudo make check
                sudo make install
    刷新动态连接库：sudo ldconfig
    查看是否安装成功：protoc --version，会输出对应的安装的protobuf的版本

>安装muduo

    下载地址：https://github.com/chenshuo/muduo 
    拷贝到虚拟机中，解压命令：uzip ***.zip
    安装相关的依赖库：
        sudo apt-get install cmake 
        sudo apt-get install libboost-dev libboost-test-dev
        下面为非必须依赖库（本项目没有安装）
        sudo apt-get install libcurl4-openssl-dev libc-ares-dev
        sudo apt-get install protobuf-compiler libprotobuf-dev
    编译安装：
        sudo ./build.sh
        sudo ./build.sh install
    测试：
        参考该文章：https://blog.csdn.net/QIANGWEIYUAN/article/details/89023980

>测试protobuf

    代码示例：
    syntax = "proto3";   // 声明protobuf的版本
    package fixbug;      // 声明了代码所在的包（类似于c++ namespace）
    // string 默认用的是bytes
    message ResultCode{
        int32 errcode = 1;  // 错误码
        bytes errmsg = 2;  // 错误信息
    }

* 执行命令：protoc test.proto --cpp_out=./
* 会生成一个LoginRequest的类，里面定义了获取错误码和错误信息的函数操作，以及修改错误码和错误信息的函数操作，定义该类对象就可以直接操作了。
* 编译命令：g++ main.cc test.pb.cc -std=c++11 -lprotobuf -pthread
* 要加上-pthread，猜测应该是对线程库有要求，如果不加上在运行的时候就会出现内核错误。

> protobuf的service服务的讲解：
以下面的protobuf代码为例子，在上述的proto文件中加上，来生成service服务类

    //定义下面选项，表示生成service服务类和rpc方法描述，默认不生成
    option cc_generic_services = true;
    service UserServiceRpc{
    rpc Login(LoginRequest) returns(LoginResponse);   // 请求和返回的类型
    }   

使用protoc生成后会生成一个UserServiceRpc类和一个UserServiceRpc_Stub的类，下面具体讲解这两个类
    
    首先继承关系是，UserServiceRpc继承自protobuf的service类，里面定义了Login的虚函数，具体的函数定义如下：
      virtual void Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done);
    其中，controller主要是做一个调用过程中错误信息的输出，request主要包含的就是远程调用方的请求（模块名+函数名+参数），而reaponse包含的就是函数返回给远程调方的信息（调用结果+成功/错误+相关标识），而done就是返回response的信息。这个函数主要是rpc的服务提供商来提供的。

    另一个类是UserServiceRpc_Stub继承自UserServiceRpc，同样也定义了Login的虚函数：
      void Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done);
    这里的说明和上述类似，这里的类主要是作用在rpc服务的调用方。其中UserServiceRpc_Stub对象的初始化需要一个channel的对象，给对象所作的功能就是调用callMethod函数：void UserServiceRpc_Stub::Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                              const ::fixbug::LoginRequest* request,
                              ::fixbug::LoginResponse* response,
                              ::google::protobuf::Closure* done) {
                        channel_->CallMethod(descriptor()->method(0),
                                        controller, request, response, done);
    }
    其中CallMethod的功能就是把rpc调用方的，模块名+函数名+参数进行protobuf的序列化，然后建立一个tcp的连接，把序列化的数据通过网络发送到rpc服务的提供方。rpc服务方接收到该数据后，就从数据中组织出调用方发起的请求（模块名+函数名+参数），然后调用rpc服务端对应的函数，最后再通过CallMethod再发送回rpc调用方

> 日志模块的编写：
    
    由于日志模块的写入是一个磁盘io的操作，为了避免其占用rpc调用的时间，这里使用数据队列+线程的方式，读写线程独立于rpc调用。

> zookeeper分布式协调服务：

    在rpc调用过程中，rpc如何知道该模块的函数调用具体在那台服务器的那个端口？zookeeper其中一个就是来解决上述问题，rpc服务的提供者会向zookeeper配置中心注册一下它提供的对应的服务（ip+port+模块名+函数），同时zk为每个服务维护一个session，通过心跳消息来保证服务节点是存活的，同时还提供了一个watcher机制，监听节点的变化，当节点发生变化救通知给客户端。
    这样rpc的调用过程就是，rpc请求某个（模块+函数+参数），通过（模块+函数）在zk中找到对应的ip+port，然后根据（ip+port）建立起与rpc服务提供者的联系，将请求发过去。

zk的安装：

    下载地址：https://zookeeper.apache.org/releases.html
    解压安装包：tar -zxvf ***.tar.gz
    修改配置文件：进入conf文件夹，修改zoo_sample.cfg中的数据路径为自定义的一个路径，默认在tmp的路径下，但tmp中的路径会重启就丢失了。
    安装java的jdk环境：sudo apt-get install openjdk-8-jdk
    进入bin文件夹，启动服务：./zkServer.sh start
    
    编译zookeeper原生的API：
    进入src/c文件夹：
    第一步：sudo  ./configure
    第二步：sudo  make
    第三步：sudo  make  install
    刷新一下动态库的配置：sudo  ldconfig





