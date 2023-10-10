
#pragma once
#include<thread>
#include "Service.h"

class Worker{
public:
    int id;
    int eachNum;
    void operator()();
    void CheckAndPutGlobal(std::shared_ptr<Service> srv);
};