#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <functional>

constexpr size_t INIT_THREAD_COUNT = 10;
constexpr size_t THRESHOLD = 1024;


//cpp 17 Any类型
class CAny
{
private:
	class BASE {
	public:
		virtual ~BASE() = default;
	};

	template <typename T>
	class CDerive : public BASE
	{
	private:
		
	public:
		CDerive(T _data) :data(_data) {}
		T data;
	};
public:
	template <typename T>
	CAny(T val) :m_data(std::make_unique<CDerive<T>>(val)) {}
	CAny() = default;
	~CAny() = default;
	CAny& operator=(const CAny&) = delete;
	CAny& operator=(CAny&&) = default;
	CAny(const CAny&) = delete;
	CAny(CAny&&) = default;
	template <typename T>
	T cast()
	{
		CDerive<T>* p = dynamic_cast<CDerive<T>*>(m_data.get());
		if (p == nullptr)
			throw "类型不匹配，转换失败";
		return p->data;
	}
private:
	std::unique_ptr<BASE> m_data;
};


//线程池模式
enum class CPollMode
{
	MODE_FIXED,		//固定数量
	MODE_CACHED		//动态数量
};
class CResult;
//任务类
class CTask
{
private:
	CResult* m_result = nullptr;
public:
	void setResult(CResult* p);
	virtual CAny CallFunction() { return nullptr; }
	void exec();
};
//class CThreadPoll;
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
	CResult AddTask(std::shared_ptr<CTask>);			//添加任务
	void Start(int count = INIT_THREAD_COUNT);			//开始
	void SetTaskQueMaxThreshold(int threshhold);		//设置上限阙值
	void CallThreadFunction();							//线程执行函数
	CThreadPoll();
	~CThreadPoll();
};

//信号量
class emaphore
{
private:
	std::mutex					m_mutex;
	std::condition_variable		m_CV;
	size_t						count;
public:
	emaphore(size_t _count = 0) :count(_count) {}
	void wait() {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_CV.wait(lock, [this]()->bool {return count > 0; });
		count--;
	}
	void post()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		count++;
		m_CV.notify_all();
	}
};

//结果类
class CResult
{
private:
	CAny						m_any;
	std::atomic_bool			m_bl;
	std::shared_ptr<CTask>		m_sp;
public:
	CResult(std::shared_ptr<CTask> sp, bool isValue = false) :m_sp(sp), m_bl(isValue) { m_sp->setResult(this); }
	CResult(const CResult& val);
	~CResult() = default;
	void SetTask(CAny any);
	CAny Get();
	emaphore					m_emaphore;
};