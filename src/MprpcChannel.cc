#include "MprpcChannel.h"
#include <string>
#include "RpcHeader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "MprpcApplication.h"
#include <arpa/inet.h>
#include <unistd.h>


/*
header_size + service_name method_name args_size + args
*/

// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据数据序列化和网络方法
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    
    std::string service_name = sd->name();
    std::string method_name = method->name();

    //获取n参数的序列化字符串n长度 args_size
    std::string args_str;
    int args_size = 0;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        // std::cout<<"serialize rq error: "<<std::endl;
        controller->SetFailed("serialize rq error!");
        return ;
    }
    //定义rpc请求的header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    std::string rpc_header_str;
    int header_size = 0;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        // std::cout<<"serialize rq error: "<<std::endl;
        controller->SetFailed("serialize rpc header error!");
        return ;
    }
    //组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char *)&header_size,4));  //header_size
    send_rpc_str += rpc_header_str;   //rpcheader
    send_rpc_str += args_str;   //args

    //打印调试信息
    std::cout<<"================================================"<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;
    std::cout<<"args_size: "<<args_size<<std::endl;
    std::cout<<"args_str: "<<args_str<<std::endl;
    std::cout<<"================================================"<<std::endl;

    //使用tcp编程完成rpc方法的远程调用
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == clientfd)
    {
        char errtext[512] = {0};
        sprintf(errtext,"create socket error ! errno:%d!",errno);
        // std::cout<<"create socket error ! errno: "<<errno<<std::endl;
        // std::cout<<errtext<<std::endl;
        controller->SetFailed(errtext);
        close(clientfd);
        // exit(EXIT_FAILURE);
        return ;
    }
    
    std::string ip  = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());

    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET ;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    int err = 0;
    if(-1 == (err = connect(clientfd,(struct sockaddr *)&server_addr,sizeof(server_addr))))
    {
        // std::cout<<"connect error ! errno: "<<errno<<std::endl;
        // close(clientfd);
        // exit(EXIT_FAILURE);
        close(clientfd);
        char errtext[512] = {0};
        sprintf(errtext,"connect error ! errno:%d!",errno);
        // std::cout<<errtext<<std::endl;
        controller->SetFailed(errtext);
        return ;
    }

    if(-1 == send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
        // std::cout<<"send error ! errno: "<<errno<<std::endl;
        // close(clientfd);
        close(clientfd);
        char errtext[512] = {0};
        sprintf(errtext,"send error ! errno:%d!",errno);
        // std::cout<<errtext<<std::endl;
        controller->SetFailed(errtext);
        return ;
    }

    //接收rpc响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if(-1 == (recv_size = recv(clientfd,recv_buf,1024,0)))
    {
        // std::cout<<"recv error ! errno: "<<errno<<std::endl;   
        // close(clientfd);
        // return ;
        close(clientfd);
        char errtext[512] = {0};
        sprintf(errtext,"recv error ! errno:%d!",errno);
        // std::cout<<errtext<<std::endl;
        controller->SetFailed(errtext);
        return ;
    }


    //反序列化rpc调用的响应数据
    // std::string rs_str(recv_buf,0,recv_size);   //string \0 的问题 遇到\0认为字符串结束导致字符串被截取
    // if(!response->ParseFromString(rs_str))
    if(!response->ParseFromArray(recv_buf,recv_size)) 
    {
        // std::cout<<"parse error ! rs_str: "<<recv_buf<<std::endl;
        // close(clientfd);
        // return ;
        close(clientfd);
        char errtext[512] = {0};
        sprintf(errtext,"parse error ! rs_str:%s!",recv_buf);
        // std::cout<<errtext<<std::endl;
        controller->SetFailed(errtext);
        return ;
    }
    close(clientfd);
}
                