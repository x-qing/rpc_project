aux_source_directory(. SRC_LIST)

# 依据源文件生成动态库
add_library(mprpc ${SRC_LIST})

# 添加依赖
# muduo的顺序也很重要
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)



