#pragma once
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
private:
    // zk客户端句柄
    zhandle_t *m_zhandle;
public:
    ZkClient();
    ~ZkClient();
    // 启动连接
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    void Create(const char *path,const char *data,int datalen,int state=0);
    // 根据参数指定的znode节点路径或者znode节点的值
    std::string GetData(const char *path);
};

