#include "mprpcconfig.h"

#include <iostream>
#include <string>


// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file){
    FILE *pf = fopen(config_file,"r");
    if(pf == nullptr){
        std::cout << config_file << "is not exits!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 1.注释
    // 2.正确的配置项
    // 3.去掉开头多余的空格
    while(!feof(pf)){
        char buf[512] = {0};
        fgets(buf,512,pf);   // 读取文件
        // 去掉字符串前面多余的空格
        std::string read_buf(buf);
        Trim(read_buf);
        // 从前往后找
        // int idx = src_buf.find_first_not_of(' ');
        // if(idx != -1){
        //     // 说明字符串前面有空格，做字符串截取
        //     src_buf = src_buf.substr(idx,src_buf.size()-idx);  // 位置和长度
        // }
        // // 从后往前找
        // idx = src_buf.find_last_not_of(' ');   // 返回下标
        // if(idx != -1){
        //     src_buf = src_buf.substr(0,idx+1); // 去掉后面的空格
        // }

        // 判断#注释
        if(read_buf.empty() || read_buf[0]=='#'){
            continue;
        }

        // 继续解析配置项
        int idx = read_buf.find('=');
        if(idx == -1){
            // 配置项不合法
            continue;
        }

        std::string key;
        std::string value;

        key = read_buf.substr(0,idx);
        Trim(key);

        int end_idx = read_buf.find('\n',idx);  // 字符的转义
        value = read_buf.substr(idx+1,end_idx-idx-1);
        Trim(value);

        m_configMap.insert({key,value});
    }
}

// 查询配置项信息
std::string MprpcConfig::Load(const std::string& key){
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()){
        return "not";
    }
    return it->second;
}


void MprpcConfig::Trim(std::string &src_buf){
    // 从前往后找
    int idx = src_buf.find_first_not_of(' ');
    if(idx != -1){
        // 说明字符串前面有空格，做字符串截取
        src_buf = src_buf.substr(idx,src_buf.size()-idx);  // 位置和长度
    }
    // 从后往前找
    idx = src_buf.find_last_not_of(' ');   // 返回下标
    if(idx != -1){
        src_buf = src_buf.substr(0,idx+1); // 去掉后面的空格
    }
}