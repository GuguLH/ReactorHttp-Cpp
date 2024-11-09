#ifndef __EPOLL_DISPATCHER_H__
#define __EPOLL_DISPATCHER_H__

#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#include "Channel.h"
#include "Eventloop.h"
#include "Dispatcher.h"

using namespace std;

class Eventloop;

class EpollDispatcher : public Dispatcher
{
public:
	EpollDispatcher(Eventloop* evloop);
	~EpollDispatcher();
	// 添加
	int add() override;
	// 删除
	int remove() override;
	// 修改
	int modify() override;
	// 事件检测
	int dispatch(int timeout = 3) override;
private:
	int epollControl(int op);
private:
	int m_epfd;
	struct epoll_event* m_events;
	const int m_maxNode = 512;
};

#endif // !__EPOLL_DISPATCHER_H__
