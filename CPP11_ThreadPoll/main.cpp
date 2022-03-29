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
    /*CThreadPoll poll;
    poll.Start();
    for (int i = 0; i < 1000; i++) {
        CResult res = poll.AddTask(std::make_shared<task>());
        CAny any = res.Get();
        std::cout << any.cast<unsigned int>() << '\n';
    }*/
    std::mutex					m_mutex;
    std::unique_lock<std::mutex> lock(m_mutex);
}
