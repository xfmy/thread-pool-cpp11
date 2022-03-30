#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <iostream>
#include <condition_variable>
#include <memory>
#include <unordered_map>
#include <functional>

constexpr size_t INIT_THREAD_COUNT			= 10;		//Ĭ�ϳ�ʼ���̳߳��߳�����
constexpr size_t THRESHOLD					= 1024;		//Ĭ�����������ֵ
constexpr size_t THREAD_SIZE_THRESH_HOLD	= 100;		//Ĭ���̳߳��߳������ֵ
constexpr int THREAD_MAX_IDLE_TIME			= 1;		//Ĭ���߳�������ʱ��s

//cpp 17 Any����
class CAny
{
private:
	//����
	class BASE {
	public:
		virtual ~BASE() = default;
	};

	//������
	template <typename T>
	class CDerive : public BASE
	{
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

	//��ȡ���ؽ��
	template <typename T>
	T cast()
	{
		CDerive<T>* p = dynamic_cast<CDerive<T>*>(m_data.get());
		if (p == nullptr)
			throw "���Ͳ�ƥ�䣬ת��ʧ��";
		return p->data;
	}
private:
	//�洢����Ļ���ָ��
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
	//������ָ��
	CResult* m_result = nullptr;
public:
	//����������
	void setResult(CResult* p);
	virtual CAny CallFunction() { return nullptr; }
	//ִ������
	void exec();
};
//class CThreadPoll;
//�߳���
class CThread
{
	using FUNTYPE = std::function<void(int)>;
public:
	CThread(FUNTYPE parameter):m_callFun(parameter), m_threadId(numbers++) 
	{
		std::cout << "create thread\n";
	}
	~CThread()
	{
		std::cout << "release thread\n";
	}
	void start();
	int Getid()const;
private:
	FUNTYPE m_callFun;										//��ִ�к�������
	static int numbers;										//�̱߳��
	int m_threadId;											//�߳�id
};

class CThreadPoll
{
private:
	size_t									m_count;		//��ʼ�߳�����
	size_t									m_taskMax;		//����������ֵ
	size_t									m_threadSizeThreshHold;//�߳�������ֵ
	std::atomic_uint						m_taskSize;		//��������
	CPollMode								m_mode;			//�̳߳�ģʽ
	//std::vector<std::unique_ptr<CThread>>	m_arr;			//�̶߳���
	std::unordered_map<int, std::unique_ptr<CThread>> m_arr;//�̶߳���
	std::queue<std::shared_ptr<CTask>>		m_que;			//�������
	std::atomic_bool						m_isConfirm;	//�Ƿ�����ȷ��
	std::mutex								m_queMut;		//������в�����
	std::condition_variable					m_notFull;		//������в���
	std::condition_variable					m_notEmpty;		//������в���
	std::atomic_uint						m_idleThreadSize;//�����߳�����
	std::atomic_int							m_currThreadSize;//��ǰ�߳�����
public:
	void SetMode(CPollMode parameter);						//����ģʽ
	CResult AddTask(std::shared_ptr<CTask>);				//�������
	void Start(int count = INIT_THREAD_COUNT);				//��ʼ
	void SetTaskQueMaxThreshold(int threshhold);			//��������������ֵ
	void SetThreadSizeThreshHold(size_t threshhold);		//�����߳�������ֵ
	void CallThreadFunction(int);							//�߳�ִ�к���
	CThreadPoll();
	~CThreadPoll();
};

//�ź���
class emaphore
{
private:
	std::mutex					m_mutex;
	std::condition_variable		m_CV;
	size_t						count;						//�ź���
public:
	emaphore(size_t _count = 0) :count(_count) {}
	//��ȡһ���ź�
	void wait() {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_CV.wait(lock, [this]()->bool {return count > 0; });
		count--;
	}
	//Ͷ��һ���ź�
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
	CAny						m_any;			//���ֵ
	std::atomic_bool			m_bl;			//�Ƿ���Ч
	std::shared_ptr<CTask>		m_sp;			//����ָ��
public:
	CResult(std::shared_ptr<CTask> sp, bool isValue = false) :m_sp(sp), m_bl(isValue) { m_sp->setResult(this); }
	CResult(const CResult& val);
	~CResult() = default;
	void SetTask(CAny any);						//���ý��
	CAny Get();									//��ȡ���
	emaphore					m_emaphore;		//�߳�֪ͨ
};