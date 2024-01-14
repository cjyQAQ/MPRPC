#include "MprpcConfig.h"
#include <string>
#include <iostream>


void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file,"r");
    if(nullptr == pf)
    {
        std::cout<<config_file<<"is not exist !"<<std::endl;
        exit(EXIT_FAILURE);
    }

    // 1. 注释   2. 正确配置项  3. 去掉a开头多余的空格
    while(!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf,512,pf);

        //去掉多余空格
        std::string readbuf(buf);
        Trim(readbuf);
        
        if (readbuf[0] == '#' || readbuf.empty())
        {
            continue;
        }

        //解析配置项
        int idx = readbuf.find('=');
        if(idx == -1)
        {
            continue;
        }

        std::string key;
        std::string value;
        key = readbuf.substr(0,idx);
        Trim(key);
        int endidx = readbuf.find('\n',idx);
        value = readbuf.substr(idx+1,endidx - idx - 1);
        Trim(value);
        m_configMap.insert({key,value});
        
    }
}

MprpcConfig::MprpcConfig(/* args */)
{
}

MprpcConfig::~MprpcConfig()
{
}

std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}



void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if(idx != -1)
    {
        //说明前面有空格
        src_buf = src_buf.substr(idx,src_buf.size() - idx);
    }
    //去掉后面多余的空格
    idx = src_buf.find_last_not_of(' ');
    if(idx != -1)
    {
        //说明前面有空格
        src_buf = src_buf.substr(0,idx + 1);
    }
    
}