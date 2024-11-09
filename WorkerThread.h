#ifndef __WORKER_THREAD_H__
#define __WORKER_THREAD_H__

#include <thread>
#include <mutex>
#include <condition_variable>

#include "Eventloop.h"

using namespace std;

class WorkerThread
{
public:
	WorkerThread(int index);
	~WorkerThread();
	void run();
	inline Eventloop* getEventLoop()
	{
		return m_evLoop;
	}

private:
	void doHandle();

private:
	thread* m_thread; // 保存线程的实例
	thread::id m_threadId;
	string m_name;
	mutex m_mutex;
	condition_variable m_cond;
	Eventloop* m_evLoop;
};

#endif // !__WORKER_THREAD_H__
