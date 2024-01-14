#include <iostream>
#include <string>
#include <user.pb.h>
#include "MprpcApplication.h"
#include "RpcProvider.h"

using namespace fixbug;
using namespace std;

/*
UserService原来是一个本地服务，提供了两个进程内的本地n方法，Login和GetFriendLists
*/
class UserService : public UserServiceRpc   //RPC服务发布端（rpc服务提供者）
{
public:
    bool Login(string name,string pwd)
    {
        cout<<"UserService Login"<<endl;
        cout<<"name: "<<name<<endl;
        cout<<"pwd: "<<pwd<<endl;
        return true;
    }
    bool Register(uint32_t id,string name,string pwd)
    {
        cout<<"UserService Register"<<endl;
        cout<<"id: "<<id<<endl;
        cout<<"name: "<<name<<endl;
        cout<<"pwd: "<<pwd<<endl;
        return true;
    }
    // 重写UserServiceRpc中的RPC的虚函数
    // 框架调用该方法
    // 框架调用流程
    // caller  ===> Login(LoginRq) => muduo => callee
    // callee  ===> Login(LoginRq) => 交到下面重写的Login方法上
    void Login(::google::protobuf::RpcController* controller,
                     const ::fixbug::LoginRq* request,
                     ::fixbug::LoginRs* response,
                     ::google::protobuf::Closure* done)
    {
       // 框架给业务上报了请求参数LoginRq，业务获取相应数据做本地业务
       string name = request->name();
       string pwd = request->pwd();

        //进行本地业务
       bool login_rs = Login(name,pwd);

        //写入响应
        ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_rs);

        //执行回调操作   执行响应对象数据的序列化和网络发送
        done->Run();   
    }
    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRq* request,
                       ::fixbug::RegisterRs* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        string name = request->name();
        string pwd = request->pwd();

        bool ret = Register(id,name,pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);

        done->Run();
        
        
    }
};

int main(int argc , char **argv)
{
    // 调用框架的初始化
    MprpcApplication::Init(argc,argv);

    //provider是一个rpc网络服务对象，把UserService对象发布到rpc节点
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();
    return 0;
}

