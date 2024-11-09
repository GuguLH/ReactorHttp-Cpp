#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <string>

#include "Channel.h"
#include "Eventloop.h"

using namespace std;

class Eventloop;

class Dispatcher
{
public:
	Dispatcher(Eventloop* evloop) : m_evLoop(evloop) {};
	virtual ~Dispatcher() {};
	// 添加
	virtual int add() = 0;
	// 删除
	virtual int remove() = 0;
	// 修改
	virtual int modify() = 0;
	// 事件检测
	virtual int dispatch(int timeout = 3) = 0;
	inline void setChannel(Channel* channel)
	{
		m_channel = channel;
	}
protected:
	string m_name = string();
	Channel* m_channel;
	Eventloop* m_evLoop;
};

#endif // !__DISPATCHER_H__
