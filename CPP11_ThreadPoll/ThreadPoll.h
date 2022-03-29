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
	virtual void CallFunction(){}
};
class CThreadPoll;
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
	bool AddTask(std::shared_ptr<CTask>);				//�������
	void Start(int count = INIT_THREAD_COUNT);			//��ʼ
	void SetTaskQueMaxThreshold(int threshhold);		//����������ֵ
	void CallThreadFunction();							//�߳�ִ�к���
	CThreadPoll();
	~CThreadPoll();
};

