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
		T data;
	public:
		CDerive(T _data) :data(_data) {}
	};
public:
	template <typename T>
	CAny(T val) :m_data(std::make_unique<CDerive<T>>(val)) {}
	CAny() = delete;
	~CAny() = default;
	CAny operator=(const CAny&) = delete;
	CAny(const CAny&) = delete;

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

//������
class CTask
{
public:
	virtual CAny CallFunction(){}
};
//class CThreadPoll;
//�߳���
class CThread
{
	using FUNTYPE = std::function<CAny()>;
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
	bool AddTask(std::shared_ptr<CTask>);				//�������
	void Start(int count = INIT_THREAD_COUNT);			//��ʼ
	void SetTaskQueMaxThreshold(int threshhold);		//����������ֵ
	void CallThreadFunction();							//�߳�ִ�к���
	CThreadPoll();
	~CThreadPoll();
};
