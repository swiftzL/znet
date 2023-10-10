#include <memory>
#pragma once
class BaseMsg
{
public:
    enum TYPE
    {
        SERVICE = 1,
        SOCKET_ACCEPT = 2,
        SOCKET_RW = 3,
    };
    uint8_t type;
    char load[99999]{};
    virtual ~BaseMsg(){};
};

class SocketAcceptMsg : public BaseMsg {
    public:
        int listenFd;
        int clientFd;

};

class SocketRwMsg : public BaseMsg {
    public:
        int fd;
        bool isRead;
        bool isWrite;
};

class ServiceMsg : public BaseMsg
{
public:
    uint32_t source;
    std::shared_ptr<char> buff;
    size_t size;
    ~ServiceMsg(){
    }
};