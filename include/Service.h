

#include<queue>
#include<memory>
#include<thread>
#include<Msg.h>
#include<string>
#include<mutex>

extern "C"{
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
};

#pragma once
class Service
{
private:
    lua_State* luaState;
public:
    uint32_t id;
    std::shared_ptr<std::string> type;
    bool isExiting;
    std::queue<std::shared_ptr<BaseMsg>> msgQueue;
    //lock
    std::mutex mutex; 
    // std::unique_lock<std::mutex> lock;
    Service(){
//没有初始化锁
    }
    ~Service();
    void OnInit();
    void OnMsg(std::shared_ptr<BaseMsg> msg);
    void OnExit();
    void PushMsg(std::shared_ptr<BaseMsg> msg);
    bool ProcessMsg();
    void ProcessMsgs(int max);

private:
    std::shared_ptr<BaseMsg> PopMsg();


public:
    bool isGlobal;
    std::mutex inGlobalLock;

};