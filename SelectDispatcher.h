#ifndef __SELECT_DISPATCHER_H__
#define __SELECT_DISPATCHER_H__

#include <string>
#include <sys/select.h>

#include "Channel.h"
#include "Eventloop.h"
#include "Dispatcher.h"

using namespace std;

struct Eventloop;

class SelectDispatcher : public Dispatcher
{
public:
	SelectDispatcher(Eventloop* evloop);
	~SelectDispatcher();
	// 添加
	int add() override;
	// 删除
	int remove() override;
	// 修改
	int modify() override;
	// 事件检测
	int dispatch(int timeout = 3) override;

private:
	void setFdSet();
	void clearFdSet();

private:
	fd_set m_readSet;
	fd_set m_writeSet;
	const int m_maxSize = 1024;
};
#endif // !__SELECT_DISPATCHER_H__

