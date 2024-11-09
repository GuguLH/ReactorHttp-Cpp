#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

#include <iostream>
#include <thread>
#include <queue>
#include <map>
#include <mutex>
#include <sys/socket.h>
#include <assert.h>

#include "Dispatcher.h"
#include "Channel.h"
#include "SelectDispatcher.h"
#include "PollDispatcher.h"
#include "EpollDispatcher.h"
#include "Log.h"

using namespace std;

class Dispatcher;

enum class ELE_TYPE :char
{
	ADD,
	DELETE,
	MODIFY
};

// 定义任务队列的节点
struct ChannelEle
{
	ELE_TYPE type;
	Channel* channel;
};

class Eventloop
{
public:
	Eventloop();
	Eventloop(const string threadName);
	~Eventloop();
	// 启动
	int run();
	// 处理激活的文件fd
	int eventActive(int fd, int event);
	// 往任务队列添加任务
	int addTask(Channel* channel, ELE_TYPE type);
	// 处理任务队列中的任务
	int processTaskQueue();
	// 处理dispatcher中的节点
	int add(Channel* channel);
	int remove(Channel* channel);
	int modify(Channel* channel);
	// 释放channel
	int destoryChannel(Channel* channel);
	// 获得线程ID
	inline thread::id getThreadId()
	{
		return m_threadId;
	}
private:
	int readLocalMsg();
	void writeLocalMsg();
private:
	bool m_isQuit;
	// 该指针指向子类实例
	Dispatcher* m_dispatcher;
	// 任务队列
	queue<ChannelEle*> m_taskQueue;
	// map
	map<int, Channel*> m_channelMap;
	// 线程ID,name,mutex
	thread::id m_threadId;
	string m_threadName;
	mutex m_mutex;
	int m_socketPair[2]; // 存储用于本地通信的fds
};

#endif