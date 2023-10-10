
#include <Service.h>
#include <iostream>
#include<Znet.h>

void Service::PushMsg(std::shared_ptr<BaseMsg> msg)
{
    std::lock_guard<std::mutex> lck2(this->mutex);
    msgQueue.push(msg);
}
Service::~Service(){
    std::cout<<"服务销毁" <<std::endl;
}

std::shared_ptr<BaseMsg> Service::PopMsg()
{
    std::cout<< &this->id <<std::endl;
    std::cout<< &this->mutex << std::endl;
    std::lock_guard<std::mutex> lck2(this->mutex);
    std::shared_ptr<BaseMsg> msg = NULL;
    {
        if (!msgQueue.empty())
        {
            msg = msgQueue.front();
            msgQueue.pop();
        }
    }
    return msg;
}

void Service::OnExit()
{
    std::cout << "on exit" << id << std::endl;
}

void Service::OnInit()
{
    std::cout << "on init" << this->id << std::endl;
}

void Service::OnMsg(std::shared_ptr<BaseMsg> msg)
{
    if(msg->type == BaseMsg::TYPE::SERVICE){
        auto service_msg = std::dynamic_pointer_cast<ServiceMsg>(msg);
        std::cout << "on msg" << id << "msg content is" <<service_msg->buff << std::endl;
        auto reply = Znet::instance->makeMsg(id,new char[99999]{'p','i','n','g','\0'},99999);
        Znet::instance->Send(service_msg->source,reply);
        return;
    }
    std::cout << "on msg" << id << std::endl;
}

bool Service::ProcessMsg()
{
    std::cout<< "process msg id:" <<this->id<<std::endl;
    std::shared_ptr<BaseMsg> msg = PopMsg();
    if (msg)
    {
        OnMsg(msg);
        return true;
    }
    else
    {
        return false;
    }
}

void Service::ProcessMsgs(int max){
    for(int i=0;i<max;i++){
        bool succ = this->ProcessMsg();
        if(!succ){
            break;
        }
    }
}


