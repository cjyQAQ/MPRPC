#include <iostream>
#include "MprpcApplication.h"
#include "user.pb.h"


int main(int argc,char **argv)
{
    // 整个程序启动后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);


    //演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // 请求
    fixbug::LoginRq Loginrq;
    Loginrq.set_name("zhangsan");
    Loginrq.set_pwd("123456");

    //响应
    fixbug::LoginRs Loginrs;
    
    //rpc方法调用，同步rpca调用 MprpcChannel::callMethod
    stub.Login(nullptr,&Loginrq,&Loginrs,nullptr);   //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpca调用完成，读调用结果
    if(0 == Loginrs.result().errcode())
    {
        std::cout<<"rpc login rs success: "<<Loginrs.sucess()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login rs error: "<<Loginrs.result().errmsg()<<std::endl;
    }

    // 演示调用远程发布的rpc方法Register
    fixbug::RegisterRq Registerrq;
    Registerrq.set_id(2000);
    Registerrq.set_name("mprpc");
    Registerrq.set_pwd("666666");

    fixbug::RegisterRs Registerrs;
    stub.Register(nullptr,&Registerrq,&Registerrs,nullptr);
    if(0 == Registerrs.result().errcode())
    {
        std::cout<<"rpc register rs success: "<<Registerrs.sucess()<<std::endl;
    }
    else
    {
        std::cout<<"rpc register rs error: "<<Registerrs.result().errmsg()<<std::endl;
    }
    return 0;
}