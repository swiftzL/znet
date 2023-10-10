#include "SocketWorker.h"
#include "iostream"
#include "unistd.h"


void SocketWorker::operator()()
{
    while(true){
        std::cout << "socket workering " << std::endl;
        usleep(200);
    }
}

void SocketWorker::Init()
{
    std::cout<< "socket worker init" <<std::endl;
}
