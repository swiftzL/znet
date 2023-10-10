#include<Znet.h>
#include "iostream"

void test() {
    std::cout<< "start test" << std::endl;
    auto pingType = std::make_shared<std::string>("ping");
    uint32_t ping1 =  Znet::instance->NewService(pingType);
    uint32_t ping2 = Znet::instance->NewService(pingType);
    uint32_t pong =  Znet::instance->NewService(pingType);

    auto msg1 = Znet::instance->makeMsg(ping1,new char[3]{'h','i','\0'},3);
    auto msg2 = Znet::instance->makeMsg(ping2,new char[5]{'h','i','i','i','\0'},5);
    Znet::instance->Send(pong,msg1);
    Znet::instance->Send(pong,msg2);

}

int main(int argc, char const *argv[])
{
    Znet * znet = new Znet();
    Znet::instance->Start();
    test();
    Znet::instance->Wait();
    return 0;
}
