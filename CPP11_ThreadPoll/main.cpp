#include <iostream>
#include "ThreadPoll.h"
std::atomic_uint i = 0;
class task : public CTask
{
    virtual CAny CallFunction()override
    {
        return i++;
    }
};

void fun(CThreadPoll* poll)
{
    for (int i = 0; i < 50; i++) {
        CResult res = poll->AddTask(std::make_shared<task>());
        ///std::this_thread::sleep_for(std::chrono::seconds(1));
        CAny any = res.Get();
        std::cout << any.cast<unsigned int>() << '\n';
    }
}

int main()
{
    CThreadPoll* poll = new CThreadPoll;
    poll->SetMode(CPollMode::MODE_CACHED);
    poll->Start(1);
    for (size_t i = 0; i < 10; i++)
    {
        std::thread th1(fun, poll);
        th1.detach();
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    //std::thread th1(fun, poll);
    //std::thread th2(fun, poll);
    //std::thread th3(fun, poll);
    //std::thread th4(fun, poll);
    //std::thread th5(fun, poll);
    //std::thread th6(fun, poll);
    //th1.join();
    //th2.join();
    //th3.join();
    //th4.join();
    //th5.join();
    //th6.join();
    delete poll;
}
