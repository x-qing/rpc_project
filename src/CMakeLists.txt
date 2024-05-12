aux_source_directory(. SRC_LIST)

# 依据源文件生成动态库
add_library(mprpc SHARED ${SRC_LIST})  



