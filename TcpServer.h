#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <arpa/inet.h>

#include "Eventloop.h"
#include "ThreadPool.h"
#include "TcpConnection.h"
#include "Log.h"

class TcpServer
{
public:
	TcpServer(unsigned short port, int threadNum);
	~TcpServer();
	// 初始化监听
	void setListener();
	// 启动服务器
	void run();

private:
	static int acceptConnection(void* arg);

private:
	int m_threadNum;
	Eventloop* m_mainLoop;
	ThreadPool* m_threadPool;
	int m_lfd;
	unsigned short m_port;
};

#endif // !__TCP_SERVER_H__
