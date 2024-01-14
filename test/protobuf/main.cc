
#include <iostream>
#include "test.pb.h"
#include <string>
using namespace std;
using namespace fixbug;

int main()
{
    LoginRs rs;
    ResultCode *rc = rs.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("登录失败");

    return 0;
}



int main1() 
{
    LoginRq rq;
    rq.set_name("zhangsan");
    rq.set_pwd("123456");

    //对象数据序列化
    string send_str;
    if(rq.SerializeToString(&send_str))
    {
        cout<<send_str<<endl;
    }

    cout<<endl;
    //反序列化
    LoginRq rqB ;
    if(rqB.ParseFromString(send_str))
    {
        cout<<rqB.name()<<endl;
        cout<<rqB.pwd()<<endl;
    }


    return 0;
}