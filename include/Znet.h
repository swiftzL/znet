#include <vector>
#include <Worker.h>
#include<unordered_map>
#include<Service.h>
#include "SocketWorker.h"
#include "Conn.h"
class Znet
{
public:
    static Znet *instance;
private:
    SocketWorker* socketWorker;
    std::thread* socketThread;
    void startSocket();
    std::unordered_map<uint32_t,std::shared_ptr<Conn>> conns;

public:
    Znet();
    void Start();
    void Wait();

    std::mutex connMutex;

    int addConn(int fd,uint32_t id,Conn::TYPE type);
    std::shared_ptr<Conn> getConn(int fd);
    void removeConn(int fd);


public:
    int WORKER_NUM = 3;
    std::vector<Worker *> workers;
    std::vector<std::thread *> worker_threads;
    void StartWorker();
    
    std::unordered_map<uint32_t,std::shared_ptr<Service>> services;
    uint32_t maxId;
    pthread_rwlock_t servicesLock;

    uint32_t NewService(std::shared_ptr<std::string> type);
    void KillService(uint32_t serviceId);

private:
    std::shared_ptr<Service> GetService(uint32_t id);
    std::queue<std::shared_ptr<Service>> globalQueue;
    int globalLen;
    std::mutex globalLock;



public:
    void Send(uint32_t id,std::shared_ptr<BaseMsg> msg);
    std::shared_ptr<Service> popGlobalQueue();
    void pushGlobalQueue(std::shared_ptr<Service> srv);
    std::shared_ptr<BaseMsg> makeMsg(uint32_t source_id,char* msg,int len);

};
