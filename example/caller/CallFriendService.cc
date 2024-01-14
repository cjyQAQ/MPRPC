#include <iostream>
#include "MprpcApplication.h"
#include "friend.pb.h"



int main(int argc,char **argv)
{
    // 整个程序启动后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);


    //演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    // 请求
    fixbug::GetFriendsListRq rq;
    rq.set_userid(1000);

    //响应
    fixbug::GetFriendsListRs rs;

    
    MprpcController controller;
    //rpc方法调用，同步rpca调用 MprpcChannel::callMethod
    stub.GetFriendsList(&controller,&rq,&rs,nullptr);   //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpca调用完成，读调用结果
    if(controller.Failed())
    {
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else
    {
        if(0 == rs.result().errcode())
        {
            std::cout<<"rpc getFriendsList rs success!"<<std::endl;
            int size = rs.friends_size();
            for(int i = 0 ;i< size;i++)
            {
                std::cout<<"index: "<< i <<" name: "<<rs.friends(i)<<std::endl;
            }
        }
        else
        {
            std::cout<<"rpc getFriendsList rs error: "<<rs.result().errmsg()<<std::endl;
        }
    }
    

    
    return 0;
}