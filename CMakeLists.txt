#版本要求和项目名称
cmake_minimum_required(VERSION 3.0)
project(mprpc)
# 指定使用c++11的标准
set(CMAKE_CXX_STANDARD 11)

# 生成debug版本
set(CMAKE_BUILD_TYPE "Debug")

# 设置项目可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#库文件路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含了rpc框架使用的相关代码
add_subdirectory(src)

# example包含相关的示例代码
add_subdirectory(example)

