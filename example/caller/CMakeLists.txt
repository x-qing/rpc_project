set(SRC_LIST  callfriendservice.cc  ../friend.pb.cc)

add_executable(consumer ${SRC_LIST})

# 这里已经把muduo库链接到框架里面了，不需要再添加muduo
target_link_libraries(consumer mprpc protobuf pthread)