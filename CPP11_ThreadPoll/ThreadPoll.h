#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <functional>

#define INIT_THREAD_COUNT	10
#define THRESHOLD			1024

//线程池模式
enum class CPollMode
{
	MODE_FIXED,		//固定数量
	MODE_CACHED		//动态数量
};

//任务类
class CTask
{
public:
	virtual void CallFunction(){}
};
class CThreadPoll;
//线程类
class CThread
{
	using FUNTYPE = std::function<void()>;
public:
	CThread(FUNTYPE parameter):m_callFun(parameter){}
	void start();
private:
	FUNTYPE m_callFun;
};

class CThreadPoll
{
private:
	size_t									m_count;	//初始线程数量
	size_t									m_taskMax;	//任务上限阙值
	std::atomic_uint						m_taskSize;	//任务数量
	CPollMode								m_mode;		//线程池模式
	std::vector<std::unique_ptr<CThread>>	m_arr;		//线程队列
	std::queue<std::shared_ptr<CTask>>		m_que;		//任务队列

	std::mutex								m_queMut;	//任务队列操作锁
	std::condition_variable					m_notFull;	//任务队列不满
	std::condition_variable					m_notEmpty;	//任务队列不空
public:
	void SetMode(CPollMode parameter);					//设置模式
	bool AddTask(std::shared_ptr<CTask>);				//添加任务
	void Start(int count = INIT_THREAD_COUNT);			//开始
	void SetTaskQueMaxThreshold(int threshhold);		//设置上限阙值
	void CallThreadFunction();							//线程执行函数
	CThreadPoll();
	~CThreadPoll();
};

