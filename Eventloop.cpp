#include "Eventloop.h"

Eventloop::Eventloop() : Eventloop(string())
{

}

Eventloop::Eventloop(const string threadName)
{
	m_isQuit = true; // 默认没有启动
	m_threadId = this_thread::get_id();
	m_threadName = threadName == string() ? "MainThread" : threadName;
	m_dispatcher = new EpollDispatcher(this);
	// map
	m_channelMap.clear();
	int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, m_socketPair);
	if (ret == -1)
	{
		perror("Error socketpair()");
		exit(EXIT_FAILURE);
	}
#if 0
	// fds: 0 send, 1 recv
	Channel* channel = new Channel(m_socketPair[1], FD_EVENT::READ_EVENT, readLocalMsg, nullptr, nullptr, this);
#else
	// 绑定 - bind()
	auto obj = bind(&Eventloop::readLocalMsg, this);
	Channel* channel = new Channel(m_socketPair[1], FD_EVENT::READ_EVENT, obj, nullptr, nullptr, this);
#endif
	// channel 添加到任务队列
	addTask(channel, ELE_TYPE::ADD);
	Debug("%s eventloop create!\n", m_threadName.c_str());
}

Eventloop::~Eventloop()
{
}

int Eventloop::run()
{
	m_isQuit = false;
	// 比较线程ID是否正常
	if (m_threadId != this_thread::get_id())
	{
		return -1;
	}
	// 循环进行事件处理
	while (!m_isQuit)
	{
		m_dispatcher->dispatch();
		processTaskQueue();
	}
	return 0;
}

int Eventloop::eventActive(int fd, int event)
{
	if (fd < 0)
	{
		return -1;
	}
	// 取出channel
	Channel* channel = m_channelMap[fd];
	assert(channel->getSocket() == fd);
	if ((event & static_cast<int>(FD_EVENT::READ_EVENT)) && channel->readCallback)
	{
		channel->readCallback(const_cast<void*>(channel->getArg()));
	}
	if ((event & static_cast<int>(FD_EVENT::WRITE_EVENT)) && channel->writeCallback)
	{
		channel->writeCallback(const_cast<void*>(channel->getArg()));
	}
	return 0;
}

int Eventloop::addTask(Channel* channel, ELE_TYPE type)
{
	// 锁
	m_mutex.lock();
	ChannelEle* ele = new ChannelEle();
	ele->channel = channel;
	ele->type = type;
	m_taskQueue.push(ele);
	m_mutex.unlock();

	// 处理节点
	/*
	细节:
	1.对于链表节点的添加: 可能是当前线程也可能时其他线程(主线程)
		a.修改fd事件,子线程发起,当前子线程处理
		b.添加新的fd,添加任务节点是由主线程发起
	2.主线程不会处理节点,只让子线程处理节点
	*/
	if (m_threadId == this_thread::get_id())
	{
		Debug("thread_name: %s\n", m_threadName.c_str());
		// 为子线程
		processTaskQueue();
	}
	else
	{
		// 主线程:告诉子线程处理任务队列中的任务
		// 1.子线程正在工作 2.子线程被阻塞了
		writeLocalMsg();
	}
	return 0;
}

int Eventloop::processTaskQueue()
{
	while (!m_taskQueue.empty())
	{
		m_mutex.lock();
		ChannelEle* node = m_taskQueue.front();
		m_taskQueue.pop();
		m_mutex.unlock();
		Channel* channel = node->channel;
		if (node->type == ELE_TYPE::ADD)
		{
			add(channel);
			Debug("Add channel, fd: %d, thread_name: %s\n", channel->getSocket(), m_threadName.c_str());
		}
		else if (node->type == ELE_TYPE::DELETE)
		{
			remove(channel);
		}
		else if (node->type == ELE_TYPE::MODIFY)
		{
			modify(channel);
		}
		delete node;
	}
	return 0;
}

int Eventloop::add(Channel* channel)
{
	int fd = channel->getSocket();
	// 找到fd对应的数组元素位置,并存储
	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		m_channelMap.insert(make_pair(fd, channel));
		m_dispatcher->setChannel(channel);
		int ret = m_dispatcher->add();
		return ret;
	}
	return -1;
}

int Eventloop::remove(Channel* channel)
{
	int fd = channel->getSocket();
	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->remove();
	return ret;
}

int Eventloop::modify(Channel* channel)
{
	int fd = channel->getSocket();
	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->modify();
	return ret;
}

int Eventloop::destoryChannel(Channel* channel)
{
	// 删除channel和fd的对应关系
	auto it = m_channelMap.find(channel->getSocket());
	if (it != m_channelMap.end())
	{
		m_channelMap.erase(it);
		close(channel->getSocket());
		delete channel;
	}
	return 0;
}

int Eventloop::readLocalMsg()
{
	char buf[64] = { 0 };
	read(m_socketPair[1], buf, sizeof(buf));
	return 0;
}

void Eventloop::writeLocalMsg()
{
	string msg = "Ping";
	int ret = write(m_socketPair[0], msg.c_str(), msg.length());
	if (ret > 0)
	{
		Debug("Main loop Send: %s success!\n", msg.c_str());
	}
}
