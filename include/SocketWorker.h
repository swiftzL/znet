#pragma once

class SocketWorker {
private:
    int epoolFd;
public:
     void Init();
    void operator()();
};
