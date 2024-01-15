#include "RpcProvider.h"
#include <MprpcApplication.h>
#include "RpcHeader.pb.h"
#include "Logger.h"



RpcProvider::RpcProvider(/* args */)
{

}
RpcProvider::~RpcProvider()
{

}

/**
 * servive_name => service 描述
 *          => service * 保存服务对象
 *          => method_name => method方法对象
*/

// 这里是框架提供给外部使用的，可以发布的rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象的方法数量
    int methodCnt = pserviceDesc->method_count();

    // std::cout<<"service_name:"<<service_name<<std::endl;
    LOG_INFO("service_name : %s",service_name.c_str());

    for(int i = 0;i<methodCnt;i++)
    {
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});
        // std::cout<<"method_name:"<<method_name<<std::endl;
        LOG_INFO("method_name : %s",method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name,service_info});
}
// 启动RPC服务节点，开始提供rpc网络调用服务函数接口
void RpcProvider::Run()
{
    std::string ip  = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());
    muduo::net::InetAddress address(ip,port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");

    // 绑定连接回调和消息读写回调方法   分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(4);
    
    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;
    //启动网络服务
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr&conn)
{
    if(!conn->connected())
    {
        // 和rpc client的连接断开
        conn->shutdown();
    }
}
/*
需要双方协商好通信用的protobuf数据类型
service_name method_name args 定义相应的protobuf的message类型，进行数据的序列化和反序列化  
service_name method_name args_size
header_size(4个字节) + header_str + args_str


*/


// 已建立连接的用户读写事件回调   如果远端有一个rpc服务的调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer *buffer,muduo::Timestamp)
{
    // 网络上收到的远程rpc调用请求的字符流 方法名字 方法参数
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size,4,0);

    // 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }     
    else
    {
        // 数据头反序列化失败
        std::cout<<"rpc_header_str:"<<rpc_header_str<<" parse error!"<<std::endl;
        return ;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4+header_size,args_size);

    //打印调试信息
    std::cout<<"================================================"<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;
    std::cout<<"args_size: "<<args_size<<std::endl;
    std::cout<<"args_str: "<<args_str<<std::endl;
    std::cout<<"================================================"<<std::endl;

    // 获取service和method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end())
    {
        std::cout<<service_name<<"is not exist!"<<std::endl;
        return ;
    }


    auto methodit = it->second.m_methodMap.find(method_name);
    if(methodit == it->second.m_methodMap.end())
    {
        std::cout<<service_name<<":"<<method_name<<"is not exist!"<<std::endl;
        return ;
    }

    // 获取service对象
    google::protobuf::Service *service = it->second.m_service;
    // 获取method对象
    const google::protobuf::MethodDescriptor *method = methodit->second;

    // 生产rpc方法调用的请求rq和响应rs
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))
    {
        std::cout<<"request parase error! content: "<<args_str<<std::endl;
        return ;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的methodg方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message *>
                                        (this,&RpcProvider::SendRpcResponse,conn,response);
    // 框架上根据远端rpc请求调用当前rpc节点上发布的方法
    service->CallMethod(method,nullptr,request,response,done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))  // response进行序列化
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送给调用方
        conn->send(response_str);
    }
    else
    {
        std::cout<<"Serialize response_str error!"<<std::endl;
    }
    conn->shutdown();  //由rpc服务方主动断开连接
}
