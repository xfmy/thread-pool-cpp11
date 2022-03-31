#include "ThreadPoll.h"
#include <iostream>
int CThread::numbers = 0;

// �����̳߳صĹ���ģʽ
void CThreadPoll::SetMode(CPollMode parameter)
{
	if(!m_isConfirm)
		m_mode = parameter;
}

// ���̳߳��ύ����    �û����øýӿڣ��������������������
//CResult CThreadPoll::AddTask(std::shared_ptr<CTask> sp)
//{
//	std::unique_lock<std::mutex> lock(m_queMut);
//	//�̵߳�ͨ��  �ȴ���������п���   wait   wait_for   wait_until
//	// �û��ύ�����������������1s�������ж��ύ����ʧ�ܣ�����
//	bool res = m_notFull.wait_for(lock, std::chrono::seconds(1), 
//		[this]()->bool { return m_que.size() < m_taskMax; });
//	if (!res) 
//		// return task->getResult();  // Task  Result   �߳�ִ����task��task����ͱ���������
//		return CResult(sp);
//	// ����п��࣬������������������
//	m_que.emplace(sp);//�������
//	m_taskSize++;
//	// ��Ϊ�·�������������п϶������ˣ���notEmpty_�Ͻ���֪ͨ���Ͽ�����߳�ִ������
//	m_notEmpty.notify_all();
//
//	// cachedģʽ ������ȽϽ��� ������С��������� ��Ҫ�������������Ϳ����̵߳��������ж��Ƿ���Ҫ�����µ��̳߳���
//	if (m_mode == CPollMode::MODE_CACHED				//��̬��ʽ
//		&& m_idleThreadSize < m_taskSize				//�����߳�С�������߳�
//		&& m_currThreadSize < m_threadSizeThreshHold)	//��ǰ�߳�����С���߳���ֵ
//	{
//		//�����µ��̶߳���
//		std::unique_ptr<CThread> thread = std::make_unique<CThread>(std::bind(&CThreadPoll::CallThreadFunction, this, std::placeholders::_1));
//		thread->start();// �����߳�
//		m_arr.emplace(thread->Getid(), std::move(thread));
//		// �޸��̸߳�����صı���
//		m_idleThreadSize++;		//����������һ
//		m_currThreadSize++;		//�߳���������һ
//	}
//	return CResult(sp, true);// ���������Result����
//}

// �����̳߳�
void CThreadPoll::Start(int count)
{
	m_isConfirm = true;// �����̳߳ص�����״̬
	// ��¼��ʼ�̸߳���
	m_count = count;
	m_currThreadSize = count;
	// �����̶߳���
	for (size_t i = 0; i < m_count; i++)
	{
		// ����thread�̶߳����ʱ�򣬰��̺߳�������thread�̶߳���
		std::unique_ptr<CThread> thread = std::make_unique<CThread>(std::bind(&CThreadPoll::CallThreadFunction, this, std::placeholders::_1));
		m_arr.emplace(thread->Getid(),std::move(thread));
	}
	// ���������߳� 
	for (int i = 0; i < m_count; i++)
	{
		m_arr[i]->start();
		m_idleThreadSize++;
	}
}

// ����task�������������ֵ
void CThreadPoll::SetTaskQueMaxThreshold(int threshhold)
{
	if(!m_isConfirm)
		m_taskMax = threshhold;
}

// �����̳߳�cachedģʽ���߳���ֵ
void CThreadPoll::SetThreadSizeThreshHold(size_t threshhold)
{
	if (!m_isConfirm && m_mode == CPollMode::MODE_CACHED)
		m_threadSizeThreshHold = threshhold;
}

// �����̺߳���   �̳߳ص������̴߳��������������������
void CThreadPoll::CallThreadFunction(int threadId)
{
	auto lastTime = std::chrono::steady_clock::now();
	while (m_taskSize != -1)// �����������ִ����ɣ��̳߳زſ��Ի��������߳���Դ
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
							m_idleThreadSize--;		//����������һ
							m_currThreadSize--;		//�߳���������һ
							return;
						}
					}
				}
			}
			else {
				// �ȴ�notEmpty����
				m_notEmpty.wait(lock, [this]()->bool {return !m_que.empty(); });
			}
			m_idleThreadSize--;
			// �����������ȡһ���������
			sp = m_que.front();
			m_que.pop();
			m_taskSize--;
			// �����Ȼ��ʣ�����񣬼���֪ͨ�������߳�ִ������
			if (!m_que.empty())
				m_notEmpty.notify_all();
			// ȡ��һ�����񣬽���֪ͨ��֪ͨ���Լ����ύ��������
			m_notFull.notify_all();
		}//�ͷ���
		if (sp != nullptr)
			sp();// ִ�����񣻰�����ķ���ֵsetVal��������Result
		m_idleThreadSize++;
		lastTime = std::chrono::steady_clock::now();// �����߳�ִ���������ʱ��
	}
}
// �̳߳ع���
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
