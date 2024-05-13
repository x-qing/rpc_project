set(SRC_LIST  calluserservice.cc  ../user.pb.cc)

add_executable(consumer ${SRC_LIST})

# 这里已经把muduo库链接到框架里面了，不需要再添加muduo
target_link_libraries(consumer mprpc protobuf pthread)