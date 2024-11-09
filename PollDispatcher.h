#ifndef __POLL_DISPATCHER_H__
#define __POLL_DISPATCHER_H__

#include <string>
#include <poll.h>

#include "Channel.h"
#include "Eventloop.h"
#include "Dispatcher.h"

using namespace std;

struct Eventloop;

class PollDispatcher : public Dispatcher
{
public:
	PollDispatcher(Eventloop* evloop);
	~PollDispatcher();
	// 添加
	int add() override;
	// 删除
	int remove() override;
	// 修改
	int modify() override;
	// 事件检测
	int dispatch(int timeout = 3) override;

private:
	int m_maxFd;
	struct pollfd *m_fds;
	const int m_maxNode = 1024;
};

#endif // !__POLL_DISPATCHER_H__
