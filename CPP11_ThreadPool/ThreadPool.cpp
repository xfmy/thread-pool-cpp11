#include "ThreadPool.h"
#include <iostream>
int CThread::numbers = 0;

// 设置线程池的工作模式
void CThreadPool::SetMode(CPollMode parameter)
{
	if(!m_isConfirm)
		m_mode = parameter;
}

// 给线程池提交任务    用户调用该接口，传入任务对象，生产任务
//CResult CThreadPool::AddTask(std::shared_ptr<CTask> sp)
//{
//	std::unique_lock<std::mutex> lock(m_queMut);
//	//线程的通信  等待任务队列有空余   wait   wait_for   wait_until
//	// 用户提交任务，最长不能阻塞超过1s，否则判断提交任务失败，返回
//	bool res = m_notFull.wait_for(lock, std::chrono::seconds(1), 
//		[this]()->bool { return m_que.size() < m_taskMax; });
//	if (!res) 
//		// return task->getResult();  // Task  Result   线程执行完task，task对象就被析构掉了
//		return CResult(sp);
//	// 如果有空余，把任务放入任务队列中
//	m_que.emplace(sp);//添加任务
//	m_taskSize++;
//	// 因为新放了任务，任务队列肯定不空了，在notEmpty_上进行通知，赶快分配线程执行任务
//	m_notEmpty.notify_all();
//
//	// cached模式 任务处理比较紧急 场景：小而快的任务 需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
//	if (m_mode == CPollMode::MODE_CACHED				//动态方式
//		&& m_idleThreadSize < m_taskSize				//空闲线程小于任务线程
//		&& m_currThreadSize < m_threadSizeThreshHold)	//当前线程数量小于线程阈值
//	{
//		//创建新的线程对象
//		std::unique_ptr<CThread> thread = std::make_unique<CThread>(std::bind(&CThreadPool::CallThreadFunction, this, std::placeholders::_1));
//		thread->start();// 启动线程
//		m_arr.emplace(thread->Getid(), std::move(thread));
//		// 修改线程个数相关的变量
//		m_idleThreadSize++;		//空闲数量加一
//		m_currThreadSize++;		//线程总数量加一
//	}
//	return CResult(sp, true);// 返回任务的Result对象
//}

// 开启线程池
void CThreadPool::Start(int count)
{
	m_isConfirm = true;// 设置线程池的运行状态
	// 记录初始线程个数
	m_count = count;
	m_currThreadSize = count;
	// 创建线程对象
	for (size_t i = 0; i < m_count; i++)
	{
		// 创建thread线程对象的时候，把线程函数给到thread线程对象
		std::unique_ptr<CThread> thread = std::make_unique<CThread>(std::bind(&CThreadPool::CallThreadFunction, this, std::placeholders::_1));
		m_arr.emplace(thread->Getid(),std::move(thread));
	}
	// 启动所有线程 
	for (int i = 0; i < m_count; i++)
	{
		m_arr[i]->start();
		m_idleThreadSize++;
	}
}

// 设置task任务队列上线阈值
void CThreadPool::SetTaskQueMaxThreshold(int threshhold)
{
	if(!m_isConfirm)
		m_taskMax = threshhold;
}

// 设置线程池cached模式下线程阈值
void CThreadPool::SetThreadSizeThreshHold(size_t threshhold)
{
	if (!m_isConfirm && m_mode == CPollMode::MODE_CACHED)
		m_threadSizeThreshHold = threshhold;
}

// 定义线程函数   线程池的所有线程从任务队列里面消费任务
void CThreadPool::CallThreadFunction(int threadId)
{
	auto lastTime = std::chrono::steady_clock::now();
	while (m_taskSize != -1)// 所有任务必须执行完成，线程池才可以回收所有线程资源
	{
		//std::shared_ptr<CTask> sp;
		CTask sp;
		{
			std::unique_lock<std::mutex> lock(m_queMut);
			if (m_mode == CPollMode::MODE_CACHED)
			{
				while (m_que.size() == 0)
				{
					if (std::cv_status::timeout == m_notEmpty.wait_for(lock, std::chrono::seconds(1)))
					{
						auto time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastTime);
						if (time.count() > THREAD_MAX_IDLE_TIME && m_currThreadSize > m_count)
						{
							m_arr.erase(threadId);
							m_idleThreadSize--;		//空闲数量减一
							m_currThreadSize--;		//线程总数量减一
							return;
						}
					}
				}
			}
			else {
				// 等待notEmpty条件
				m_notEmpty.wait(lock, [this]()->bool {return !m_que.empty(); });
			}
			m_idleThreadSize--;
			// 从任务队列种取一个任务出来
			sp = m_que.front();
			m_que.pop();
			m_taskSize--;
			// 如果依然有剩余任务，继续通知其它得线程执行任务
			if (!m_que.empty())
				m_notEmpty.notify_one();
			// 取出一个任务，进行通知，通知可以继续提交生产任务
			m_notFull.notify_one();
		}//释放锁
		if (sp != nullptr)
			sp();// 执行任务；把任务的返回值setVal方法给到Result
		m_idleThreadSize++;
		lastTime = std::chrono::steady_clock::now();// 更新线程执行完任务的时间
	}
}
// 线程池构造
CThreadPool::CThreadPool()
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

CThreadPool::~CThreadPool()
{
	m_taskSize = -1;
}




////////////////////////////////////////////////////////////////////////
void CThread::start() {
	std::thread th(m_callFun, Getid());
	th.detach();
}

int CThread::Getid() const
{
	return m_threadId;
}

//void CTask::setResult(CResult* p)
//{
//	m_result = p;
//}

//void CTask::exec()
//{
//	if (m_result != nullptr) {
//		m_result->SetTask(CallFunction());
//	}
//}
//
//CResult::CResult(const CResult& temp)
//{
//	m_sp = temp.m_sp;
//	m_any = std::move(const_cast<CResult*>(&temp)->m_any);
//	m_bl = temp.m_bl.load();
//}
//
//void CResult::SetTask(CAny any)
//{
//	m_any = std::move(any);
//	m_emaphore.post();
//}
//
//CAny CResult::Get()
//{
//	if (!m_bl) return "";
//	m_emaphore.wait();
//	return std::move(m_any);
//}
