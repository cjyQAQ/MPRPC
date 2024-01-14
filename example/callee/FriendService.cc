#include <iostream>
#include <string>
#include <friend.pb.h>
#include "MprpcApplication.h"
#include "RpcProvider.h"
#include <vector>


using namespace fixbug;
using namespace std;

class FriendService : public FriendServiceRpc
{
public:
    vector<string> GetFriendsList(uint32_t userid)
    {
        cout<<"FriendService GetFriendsList"<<endl;
        cout<<"userid: "<<userid<<endl;
        vector<string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu song");
        vec.push_back("huo huo");
        return vec;
    }
    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRq* request,
                       ::fixbug::GetFriendsListRs* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();
        vector<string> friendList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(string &name : friendList)
        {
            string *p = response->add_friends();
            *p = name;
        }
        done->Run();
    }
};

int main(int argc , char **argv)
{
    // 调用框架的初始化
    MprpcApplication::Init(argc,argv);

    //provider是一个rpc网络服务对象，把UserService对象发布到rpc节点
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();
    return 0;
}