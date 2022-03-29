#include "ThreadPoll.h"
#include <iostream>
void CThreadPoll::SetMode(CPollMode parameter)
{
	if(!m_isConfirm)
		m_mode = parameter;
}

CResult CThreadPoll::AddTask(std::shared_ptr<CTask> sp)
{
	std::unique_lock<std::mutex> lock(m_queMut);
	//如果任务队列超过1s上阕，则返回
	bool res = m_notFull.wait_for(lock, std::chrono::seconds(1), 
		[this]()->bool { return m_que.size() < m_taskMax; });
	if (!res) 
		return CResult(sp);
	m_que.emplace(sp);//添加任务
	m_taskSize++;
	m_notEmpty.notify_all();

	if (m_mode == CPollMode::MODE_CACHED				//动态方式
		&& m_idleThreadSize < m_taskSize				//空闲线程小于任务线程
		&& m_currThreadSize < m_threadSizeThreshHold)	//当前线程数量小于线程阈值
	{
		// TODO 动态添加线程
		m_arr.emplace_back(std::make_unique<CThread>(std::bind(&CThreadPoll::CallThreadFunction, this)));
		m_arr.back()->start();
		m_idleThreadSize++;		//空闲数量加一
		m_currThreadSize++;		//线程总数量加一
	}
	return CResult(sp, true);
}

void CThreadPoll::Start(int count)
{
	m_isConfirm = true;
	m_count = count;
	m_currThreadSize = count;
	for (size_t i = 0; i < m_count; i++)
	{
		m_arr.emplace_back(std::make_unique<CThread>(std::bind(&CThreadPoll::CallThreadFunction,this)));
	}
	for (size_t i = 0; i < m_count; i++)
	{
		m_arr[i]->start();
		m_idleThreadSize++;
	}
}

void CThreadPoll::SetTaskQueMaxThreshold(int threshhold)
{
	if(!m_isConfirm)
		m_taskMax = threshhold;
}

void CThreadPoll::SetThreadSizeThreshHold(size_t threshhold)
{
	if (!m_isConfirm && m_mode == CPollMode::MODE_CACHED)
		m_threadSizeThreshHold = threshhold;
}

void CThreadPoll::CallThreadFunction()
{
	while (m_taskSize != -1)
	{
		std::shared_ptr<CTask> sp;
		{
			std::unique_lock<std::mutex> lock(m_queMut);
			m_notEmpty.wait(lock, [this]()->bool {return !m_que.empty(); });
			m_idleThreadSize--;
			sp = m_que.front();
			m_que.pop();
			m_taskSize--;
			if (!m_que.empty())
				m_notEmpty.notify_all();
			m_notFull.notify_all();
		}//释放锁
		if (sp != nullptr)
			sp->exec();
		m_idleThreadSize++;
	}
}

CThreadPoll::CThreadPoll()
	:m_count(INIT_THREAD_COUNT)
	, m_taskMax(THRESHOLD)
	, m_taskSize(0)
	, m_mode(CPollMode::MODE_FIXED)
	, m_isConfirm(false)
	, m_threadSizeThreshHold(THREAD_SIZE_THRESH_HOLD)
	, m_idleThreadSize(0)
	, m_currThreadSize(0)
{
}

CThreadPoll::~CThreadPoll()
{
	m_taskSize = -1;
}




////////////////////////////////////////////////////////////////////////
void CThread::start() {
	std::thread th(m_callFun);
	th.detach();
}

void CTask::setResult(CResult* p)
{
	m_result = p;
}

void CTask::exec()
{
	if (m_result != nullptr) {
		m_result->SetTask(CallFunction());
	}
}

CResult::CResult(const CResult& temp)
{
	m_sp = temp.m_sp;
	m_any = std::move(const_cast<CResult*>(&temp)->m_any);
	m_bl = temp.m_bl.load();
}

void CResult::SetTask(CAny any)
{
	m_any = std::move(any);
	m_emaphore.post();
}

CAny CResult::Get()
{
	if (!m_bl) return "";
	m_emaphore.wait();
	return std::move(m_any);
}
