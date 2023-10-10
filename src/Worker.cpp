#include<iostream>
#include<unistd.h>
#include<Worker.h>
#include<Znet.h>

void Worker::operator()(){
    while (true)
    {
        auto srv = Znet::instance->popGlobalQueue();
        if(!srv){
            usleep(100);
            continue;
        }
        srv->ProcessMsgs(eachNum);
        CheckAndPutGlobal(srv);
    }
    

}

void Worker::CheckAndPutGlobal(std::shared_ptr<Service> srv){
    if(srv->isExiting){
        return;
    }


    std::lock_guard<std::mutex> lck2(srv->mutex);
    if(!srv->msgQueue.empty()){
        Znet::instance->pushGlobalQueue(srv);
    }else{
        srv->isGlobal = false;
    }
}