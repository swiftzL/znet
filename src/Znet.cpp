
#define _XOPEN_SOURCE 500
#include<iostream>
#include<Znet.h>
#include "sys/socket.h"
#include "fcntl.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "unistd.h"

Znet* Znet::instance;

void Znet::startSocket()
{
     this->socketWorker = new SocketWorker();
     socketWorker->Init();
     socketThread = new std::thread(*socketWorker);
    
}

Znet::Znet()
{
    instance = this;
    std::cout<< this->worker_threads.size()<<std::endl;
    pthread_rwlock_init(&this->servicesLock,NULL);
}

void Znet::StartWorker() {
    for (int i = 0; i < this->WORKER_NUM; i++)
    {
        std::cout<< "worker thread is:" << i<<std::endl;
        Worker* worker = new Worker();
        worker->id = i;
        worker->eachNum = 2<<i;
        std::thread * t = new std::thread(*worker);
        workers.push_back(worker);
        worker_threads.push_back(t);
    }
}

void Znet::Wait() {
    if(worker_threads[0]) {
        worker_threads[0]->join();
    }
}

int Znet::addConn(int fd, uint32_t id, Conn::TYPE type)
{
    auto conn = std::make_shared<Conn>();
    conn->fd = fd;
    conn->service_id = id;
    conn->type = type;
    std::lock_guard<std::mutex>(this->connMutex);
    this->conns.emplace(fd,conn);
    return 1;
}

std::shared_ptr<Conn> Znet::getConn(int fd)
{
    std::lock_guard<std::mutex>(this->connMutex);
    auto conn = this->conns.find(fd);
    return conn == conns.end() ? NULL:conn->second;
}

void Znet::removeConn(int fd)
{
    std::lock_guard<std::mutex>(this->connMutex);
    conns.erase(fd);
}

void Znet::Start() {
    std::cout<<"hello" <<std::endl;
    StartWorker();
    startSocket();
}

uint32_t Znet::NewService(std::shared_ptr<std::string> type){
    auto srv = std::make_shared<Service>();
    srv->type = type;
    pthread_rwlock_wrlock(&this->servicesLock);
    {
        srv->id = this->maxId;
        this->maxId++;
        services.emplace(srv->id,srv);
    }
    pthread_rwlock_unlock(&this->servicesLock);
    srv->OnInit();
    return srv->id;
}

std::shared_ptr<Service> Znet::GetService(uint32_t id){
    std::shared_ptr<Service> srv = NULL;
    pthread_rwlock_rdlock(&servicesLock);
    {
        auto  iter = services.find(id);
        if (iter != services.end())
        {
            srv = iter->second;
        }
    }
    pthread_rwlock_unlock(&servicesLock);
    return srv;
}

void Znet::KillService(uint32_t id){
    auto srv = GetService(id);
    if(!srv){
        return;
    }
    srv->OnExit();
    srv->isExiting = true;
    pthread_rwlock_wrlock(&servicesLock);
    services.erase(id);
    pthread_rwlock_unlock(&servicesLock);

}

std::shared_ptr<Service> Znet::popGlobalQueue() {
    std::shared_ptr<Service> srv = NULL;
    std::lock_guard<std::mutex> lck2(this->globalLock);
    {
        if (!this->globalQueue.empty())
        {
            srv = this->globalQueue.front();
            this->globalQueue.pop();
            this->globalLen--;
        }
    }
    return srv;
}

void Znet::pushGlobalQueue(std::shared_ptr<Service> srv) {

    std::lock_guard<std::mutex> lck2(this->globalLock);
    {
        globalQueue.push(srv);
        globalLen++;
    }
}

void Znet::Send(uint32_t id,std::shared_ptr<BaseMsg> msg) {
    auto srv = GetService(id);
    if (!srv)
    {
        std::cout<<"send fial" << id << std::endl;
        return;
    }
    srv->PushMsg(msg);
    bool hasPush = false;
    std::lock_guard<std::mutex> lck2(srv->inGlobalLock);
    {
        if(!srv->isGlobal){
            pushGlobalQueue(srv);
            srv->isGlobal = true;
            hasPush = true;
        }
    }
}
std::shared_ptr<BaseMsg> Znet::makeMsg(uint32_t source_id,char* buff,int len){
    auto msg = std::make_shared<ServiceMsg>();
    msg->type = BaseMsg::TYPE::SERVICE;
    msg->source = source_id;
    msg->buff = std::shared_ptr<char>(buff);
    msg->size = len;
    return msg;
}


int Znet::listen_service(uint32_t port,uint32_t service_id){
    int listenFd = socket(AF_INET,SOCK_STREAM,0);
    if(listenFd <= 0){
        std::cout<< "create socket is error " <<std::endl;
        return -1;
    }
    fcntl(listenFd,F_SETFL,O_NONBLOCK);
    sockaddr_in addr;   //创建地址结构
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind
    int bind_r = bind(listenFd,(sockaddr*)&addr,sizeof(addr));
    if(bind_r == -1) {
        std::cout << "bind err " <<std::endl;
        return -1;
    }
    int lr = listen(listenFd,64);
    if(lr == -1){
        std::cout << "listen err" <<std::endl;
        return -1;
    }
    addConn(listenFd,service_id,Conn::LISTEN);
    socketWorker->addEvent(listenFd);
    return listenFd;
}

void Znet::closeFd(uint32_t fd){
    removeConn(fd);
    close(fd);
    socketWorker->removeEvent(fd);
}