#pragma once
#include "sys/epoll.h"
#include <memory>
#include "Conn.h"
class SocketWorker {
private:
    int epoolFd;
public:
     void Init();
    void operator()();
public:
    void addEvent(int fd);
    void removeEvent(int fd);
    void modifyEvent(int fd,bool epollOut);

private:
    void onEvent(epoll_event e);
    void onAccept(std::shared_ptr<Conn> c);
    void onRW(std::shared_ptr<Conn> conn,bool r,bool w);
};
