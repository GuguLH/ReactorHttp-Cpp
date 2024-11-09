#include "TcpServer.h"

TcpServer::TcpServer(unsigned short port, int threadNum)
{
	m_port = port;
	m_mainLoop = new Eventloop();
	m_threadNum = threadNum;
	m_threadPool = new ThreadPool(m_mainLoop, threadNum);
	setListener();
}

TcpServer::~TcpServer()
{

}

void TcpServer::setListener()
{
	m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_lfd == -1)
	{
		perror("Error socket()");
		return;
	}
	int opt = 1;
	int ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1)
	{
		perror("Error setsockopt()");
		return;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(m_lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror("Error bind()");
		return;
	}
	ret = listen(m_lfd, 128);
	if (ret == -1)
	{
		perror("Error listen()");
		return;
	}
}

void TcpServer::run()
{
	Debug("服务器已经启动了...");
	// 启动线程池
	m_threadPool->run();
	// 添加检测任务
	// 初始化一个channel
	// acceptConnection: 可以不使用static标识,则需要使用可调用对象绑定器来实现
	Channel* channel = new Channel(m_lfd, FD_EVENT::READ_EVENT, acceptConnection, nullptr, nullptr, this);
	m_mainLoop->addTask(channel, ELE_TYPE::ADD);
	// 启动反应堆模型
	Debug("Start mainLoop---\n");
	m_mainLoop->run();
}

int TcpServer::acceptConnection(void* arg)
{
	TcpServer* server = (TcpServer*)arg;
	// 和客户端建立连接
	int cfd = accept(server->m_lfd, NULL, NULL);
	// 从线程池取出一个子线程的反应堆模型,处理客户端连接请求
	Eventloop* evLoop = server->m_threadPool->takeWorkerEventLoop();
	// 将cfd放到tcp_connection中处理
	new TcpConnection(cfd, evLoop);
	return 0;
}
