#include "ThreadPoll.h"
#include <iostream>
void CThreadPoll::SetMode(CPollMode parameter)
{
	m_mode = parameter;
}

bool CThreadPoll::AddTask(std::shared_ptr<CTask> sp)
{
	std::unique_lock<std::mutex> lock(m_queMut);
	//如果任务队列超过1s上阕，则返回
	bool res = m_notFull.wait_for(lock, std::chrono::seconds(1), 
		[this]()->bool { return m_que.size() < m_taskMax; });
	if (!res) 
		return false;
	m_que.emplace(sp);//添加任务
	m_taskSize++;
	m_notFull.notify_all();
	return true;
}

void CThreadPoll::Start(int count)
{
	m_count = count;
	for (size_t i = 0; i < m_count; i++)
	{
		m_arr.emplace_back(std::make_unique<CThread>(std::bind(&CThreadPoll::CallThreadFunction,this)));
	}
	for (size_t i = 0; i < m_count; i++)
	{
		m_arr[i]->start();
	}
}

void CThreadPoll::SetTaskQueMaxThreshold(int threshhold)
{
	m_taskMax = threshhold;
}

void CThreadPoll::CallThreadFunction()
{
	while (true)
	{
		std::shared_ptr<CTask> sp;
		{
			std::unique_lock<std::mutex> lock(m_queMut);
			m_notEmpty.wait(lock, [this]()->bool {return !m_que.empty(); });
			sp = m_que.front();
			m_que.pop();
			m_taskSize--;
			if (!m_que.empty())
				m_notEmpty.notify_all();
			m_notFull.notify_all();
		}//释放锁
		if (sp != nullptr)
			sp->CallFunction();
	}
}

CThreadPoll::CThreadPoll()
	:m_count(INIT_THREAD_COUNT)
	, m_taskMax(THRESHOLD)
	, m_taskSize(0)
	, m_mode(CPollMode::MODE_FIXED)
{
}

CThreadPoll::~CThreadPoll()
{
	//TODO 队列释放
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}




////////////////////////////////////////////////////////////////////////
void CThread::start() {
	std::thread th(m_callFun);
	th.detach();
}
