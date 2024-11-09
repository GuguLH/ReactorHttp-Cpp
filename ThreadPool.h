#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <thread>
#include <vector>

#include "Eventloop.h"
#include "WorkerThread.h"

using namespace std;

class ThreadPool
{
public:
	ThreadPool(Eventloop* mainLoop, int count);
	~ThreadPool();
	// 启动线程池
	void run();
	Eventloop* takeWorkerEventLoop();
private:
	Eventloop* m_mainLoop;
	bool m_isStart;
	int m_threadNum;
	vector<WorkerThread*> m_workerThreads;
	int m_index;
};

#endif // !__THREAD_POOL_H__
