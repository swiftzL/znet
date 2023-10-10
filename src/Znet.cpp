#include<iostream>
#include<Znet.h>

Znet* Znet::instance;

void Znet::startSocket()
{
     this->socketWorker = new SocketWorker();
     socketWorker->Init();
     socketThread = new std::thread(*socketThread);
    
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
    return conns.erase(fd)==1;
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
