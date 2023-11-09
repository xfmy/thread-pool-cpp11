#include <iostream>
#include "threadPool.h"
std::atomic_uint i = 0;
//class task : public CTask
//{
//    virtual CAny CallFunction()override
//    {
//        return i++;
//    }
//};
//
//void fun(CThreadPoll* poll)
//{
//    for (int i = 0; i < 50; i++) {
//        CResult res = poll->AddTask(std::make_shared<task>());
//        ///std::this_thread::sleep_for(std::chrono::seconds(1));
//        CAny any = res.Get();
//        std::cout << any.cast<unsigned int>() << '\n';
//    }
//}

int fun(int a)
{
    return a;
}

int main()
{
    CThreadPool* poll = new CThreadPool;
    poll->SetMode(CPoolMode::MODE_CACHED);
    poll->Start(1);
    for (size_t i = 0; i < 10; i++)
    {
        std::future<int> res = poll->AddTask(fun, i);
        std::cout << res.get() << std::endl;
    }
    //for (size_t i = 0; i < 10; i++)
    //{
    //    std::thread th1(fun, i);
    //    th1.detach();
    //}
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    delete poll;
}
