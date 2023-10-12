#include "SocketWorker.h"
#include "iostream"
#include "unistd.h"
#include "assert.h"
#include "Znet.h"
#include "fcntl.h"
#include "sys/socket.h"


void SocketWorker::operator()()
{
    while(true){
        std::cout << "socket workering " << std::endl;
        const int EVENT_SIZE = 64;
        epoll_event events[EVENT_SIZE];
        int event_count = epoll_wait(epoolFd,events,EVENT_SIZE,-1);
        for(int i=0;i<event_count;i++){
            onEvent(events[i]);
        }
    }
}

void SocketWorker::onEvent(epoll_event e){
    int fd = e.data.fd;
    auto conn = Znet::instance->getConn(fd);
    if(conn == NULL){
        std::cout << "conn is not found" <<std::endl;
        return;
    }
    uint32_t events = e.events;
    bool isRead = events & EPOLLIN;
    bool isWirte = events & EPOLLOUT;
    bool isError = events & EPOLLERR;
    if(conn->type == Conn::LISTEN){
        if(isRead){
            onAccept(conn);
        }
    }
    if(conn->type == Conn::CLIENT){
        if(isRead || isWirte){
            onRW(conn,isRead,isWirte);
        }
        if(isError){
            std::cout << "epoll is error " <<std::endl;
        }
    }
}

void SocketWorker::onAccept(std::shared_ptr<Conn> conn){
    std::cout << "on accept fd :" << conn->fd <<std::endl;
    int clientFd = accept(conn->fd,NULL,NULL);
    if(clientFd <= 0){
        std::cout << "accept is error " << std::endl;
        return;
    }
    fcntl(clientFd,F_SETFD,O_NONBLOCK);
    Znet::instance->addConn(clientFd,conn->service_id,Conn::TYPE::CLIENT);
    epoll_event e;
    e.data.fd = clientFd;
    e.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(this->epoolFd,EPOLL_CTL_ADD,clientFd,&e) == -1){
        std::cout<< "accept epoll ctl is error clientFd:"<<clientFd <<std::endl; 
    }
    auto msg = std::make_shared<SocketAcceptMsg>();
    msg->clientFd = clientFd;
    msg->type = BaseMsg::TYPE::SOCKET_ACCEPT;
    msg->listenFd = conn->fd;
    Znet::instance->Send(conn->service_id,msg);
}

void SocketWorker::onRW(std::shared_ptr<Conn> conn,bool r,bool w){
    std::cout << "on Rw " <<std::endl;
    auto msg = std::make_shared<SocketRwMsg>();
    msg->isRead = r;
    msg->isWrite = w;
    msg->type = BaseMsg::TYPE::SOCKET_RW;
    msg->fd = conn->fd;
    Znet::instance->Send(conn->service_id,msg);

}

void SocketWorker::Init()
{
    epoolFd  = epoll_create(1024);
    assert(epoolFd > 0);
    std::cout<< "socket worker init" <<std::endl;
}

void SocketWorker::addEvent(int fd){
    std::cout<< "add evet fd:"<<fd<<std::endl;
    epoll_event e;
    e.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(epoolFd,EPOLL_CTL_ADD,fd,&e) == -1){
        std::cout<< "add event fial "<<fd<<std::endl;
    }
}

void SocketWorker::removeEvent(int fd)
{
}

void SocketWorker::modifyEvent(int fd,bool epollOut){
    std::cout<<"modify Event fd "<<fd <<" epollOut " << epollOut <<std::endl;
    epoll_event e;
    e.data.fd = fd;
    if(epollOut){
        e.events = EPOLLIN | EPOLLOUT | EPOLLET;
    }else{
        e.events = EPOLLIN |  EPOLLET;
    }
    epoll_ctl(epoolFd,EPOLL_CTL_MOD,fd,&e);
}
