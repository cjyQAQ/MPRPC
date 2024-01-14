#include "MprpcApplication.h"
#include <iostream>
#include <string>
#include <unistd.h>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp()
{
    std::cout<<"format : command -i <configfile>"<<std::endl;
}

MprpcApplication::MprpcApplication()
{

}

MprpcApplication::~MprpcApplication()
{

}

void MprpcApplication::Init(int argc, char **argv)
{
    if(argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while((c = getopt(argc,argv,"i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            // std::cout<<"invalid args!"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            // std::cout<<"need <config> !"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    //加载配置文件 rpcserver_ip= rpcserver_port= zookeeper_ip= zookeeper_port=

    m_config.LoadConfigFile(config_file.c_str());
    // std::cout<< "rpcserver_ip = " << m_config.Load("rpcserver_ip")<<std::endl;
    // std::cout<< "rpcserver_port = " << m_config.Load("rpcserver_port")<<std::endl;
    // std::cout<< "zookeeper_ip = " << m_config.Load("zookeeper_ip")<<std::endl;
    // std::cout<< "zookeeper_port = " << m_config.Load("zookeeper_port")<<std::endl;
}

MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}


MprpcConfig &MprpcApplication::GetConfig()
{
    return m_config;
}