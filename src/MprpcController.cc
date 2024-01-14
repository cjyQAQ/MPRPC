#include "MprpcController.h"

MprpcController::MprpcController()
{
    m_failed = false;
    m_errtext = "";
}

void MprpcController::Reset()
{
    m_failed = false;
    m_errtext = "";
}

bool MprpcController::Failed() const
{
    return m_failed;
}

std::string MprpcController::ErrorText() const
{
    return m_errtext;
}

void MprpcController::SetFailed(const std::string &reason)
{
    m_failed = true;
    m_errtext = reason;
}

// 暂时未实现功能
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const{return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback){}


