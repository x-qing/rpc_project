
# set(SRC_LIST  userservice.cc  ../user.pb.cc)
set(SRC_LIST  friendservice.cc  ../friend.pb.cc)
add_executable(provider ${SRC_LIST})

# 这里已经把muduo库链接到框架里面了，不需要再添加muduo
target_link_libraries(provider mprpc protobuf pthread)