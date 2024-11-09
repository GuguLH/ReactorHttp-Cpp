#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <functional>

using namespace std;

// 定义指针
// typedef int (*handleFunc)(void* arg);
// using handleFunc = int(*)(void*);

// 强类型枚举
enum class FD_EVENT
{
	READ_EVENT = 0x02,
	WRITE_EVENT = 0x04
};

// 可调用对象包装器包装的是什么? 1.函数指针 2.可调用对象(可以像函数一样调用)
// 最终得到了地址,但是没有调用

class Channel
{
public:
	using handleFunc = function<int(void*)>;
	// 初始化
	Channel(int fd, FD_EVENT events, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryFunc, void* arg);
	// 修改fd的写事件
	void writeEventEnable(bool flag);
	// 判断是否要检测写事件
	bool isWriteEventEnable();
	// 回调函数
	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destoryCallback;
	// 取出私有成员的值
	inline int getEvent()
	{
		return m_events;
	}
	inline int getSocket()
	{
		return m_fd;
	}
	inline const void* getArg()
	{
		return m_arg;
	}
private:
	// 文件描述符
	int m_fd;
	// 事件
	int m_events;
	// 回调函数的参数
	void* m_arg;
};

#endif
