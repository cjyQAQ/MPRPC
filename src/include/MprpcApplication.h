#pragma once
#include "MprpcConfig.h"
#include "MprpcChannel.h"
#include "MprpcController.h"

//mprpc框架的l初始化类
class MprpcApplication
{
private:
    /* data */
    static MprpcConfig m_config;
public:
    static void Init(int argc,char **argv);
    static MprpcApplication &GetInstance();
    //获得配置
    static MprpcConfig &GetConfig();
private:
    MprpcApplication();
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(const MprpcApplication &&) = delete;
    ~MprpcApplication();
};

