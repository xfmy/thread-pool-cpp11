#include <iostream>
#include "ThreadPoll.h"
std::atomic_uint i = 0;
class task : public CTask
{
    
    virtual void CallFunction()override
    {
        std::cout << i++ << "\n";
    }
};

int main()
{
    CThreadPoll poll;
    poll.SetMode(CPollMode::MODE_FIXED);
    poll.Start(100);
    //std::shared_ptr<CTask*> sp = new task;
    clock_t time = clock();
    for (size_t i = 0; i < 5000; i++)
    poll.AddTask(std::make_shared<task>());
    std::this_thread::sleep_for(std::chrono::seconds(5));
    //std::cout << (clock() - time) / 1000;
    //std::this_thread::sleep_for(std::chrono::seconds(1));
}
