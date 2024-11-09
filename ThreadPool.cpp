#include "ThreadPool.h"

ThreadPool::ThreadPool(Eventloop* mainLoop, int count)
{
	m_index = 0;
	m_isStart = false;
	m_mainLoop = mainLoop;
	m_threadNum = count;
	m_workerThreads.clear();
}

ThreadPool::~ThreadPool()
{
	for (auto item : m_workerThreads)
	{
		delete item;
	}
}

void ThreadPool::run()
{
	assert(!m_isStart);
	if (m_mainLoop->getThreadId() != this_thread::get_id())
	{
		exit(EXIT_FAILURE);
	}
	m_isStart = true;
	if (m_threadNum > 0)
	{
		for (int i = 0; i < m_threadNum; i++)
		{
			WorkerThread* subThread = new WorkerThread(i);
			subThread->run();
			m_workerThreads.push_back(subThread);
		}
	}
}

Eventloop* ThreadPool::takeWorkerEventLoop()
{
	assert(m_isStart);
	if (m_mainLoop->getThreadId() != this_thread::get_id())
	{
		exit(EXIT_FAILURE);
	}
	// 从线程池中找一个子线程,然后取出里面的反应堆实例
	Eventloop* evLoop = m_mainLoop;
	if (m_threadNum > 0)
	{
		evLoop = m_workerThreads[m_index]->getEventLoop();
		m_index = ++m_index % m_threadNum;
	}
	return evLoop;
}
