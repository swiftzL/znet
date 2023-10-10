
#include "iostream"
#include "memory"

class T {
public:
    ~T() {
        std::cout<< "析构t"<<std::endl;
    }
};

class A{
    public:
        T* t;
        std::mutex mutex;
        ~A() {
            std::cout<< "析构A" <<std::endl;
        }
};
void func(){
    A* a = new A();
    a->t = new T();
    delete a;
}
int main(int argc, char const *argv[])
{
    func();
    std::cout<<"func done "<<std::endl;
    A* a = new A();
    std::lock_guard<std::mutex> lock(a->mutex);
    std::lock_guard<std::mutex> lock2(a->mutex);
    return 0;
}
