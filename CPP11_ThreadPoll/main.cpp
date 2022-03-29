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

int main()
{
    CThreadPoll poll;
    poll.SetMode(CPollMode::MODE_FIXED);
    poll.Start();
    for (size_t i = 0; i < 1000; i++)
    poll.AddTask(std::make_shared<task>());
    std::this_thread::sleep_for(std::chrono::seconds(5));
}
