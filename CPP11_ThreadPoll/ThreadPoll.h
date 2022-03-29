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


//cpp 17 Any����
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
			throw "���Ͳ�ƥ�䣬ת��ʧ��";
		return p->data;
	}
private:
	std::unique_ptr<BASE> m_data;
};


//�̳߳�ģʽ
enum class CPollMode
{
	MODE_FIXED,		//�̶�����
	MODE_CACHED		//��̬����
};
class CResult;
//������
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
//�߳���
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
	size_t									m_count;	//��ʼ�߳�����
	size_t									m_taskMax;	//����������ֵ
	std::atomic_uint						m_taskSize;	//��������
	CPollMode								m_mode;		//�̳߳�ģʽ
	std::vector<std::unique_ptr<CThread>>	m_arr;		//�̶߳���
	std::queue<std::shared_ptr<CTask>>		m_que;		//�������

	std::mutex								m_queMut;	//������в�����
	std::condition_variable					m_notFull;	//������в���
	std::condition_variable					m_notEmpty;	//������в���
public:
	void SetMode(CPollMode parameter);					//����ģʽ
	CResult AddTask(std::shared_ptr<CTask>);			//�������
	void Start(int count = INIT_THREAD_COUNT);			//��ʼ
	void SetTaskQueMaxThreshold(int threshhold);		//����������ֵ
	void CallThreadFunction();							//�߳�ִ�к���
	CThreadPoll();
	~CThreadPoll();
};

//�ź���
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

//�����
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