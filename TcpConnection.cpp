#include "TcpConnection.h"

TcpConnection::TcpConnection(int fd, Eventloop* evLoop)
{
	m_evLoop = evLoop;
	m_readBuf = new Buffer(10240);
	m_writeBuf = new Buffer(10240);
	m_request = new HttpRequest();
	m_response = new HttpResponse();
	m_name = "Connection-" + to_string(fd);
	m_channel = new Channel(fd, FD_EVENT::READ_EVENT, processRead, processWrite, tcpConnDestory, this);

	evLoop->addTask(m_channel, ELE_TYPE::ADD);
}

TcpConnection::~TcpConnection()
{
	if (m_readBuf && m_readBuf->readableSize() == 0 &&
		m_writeBuf && m_writeBuf->readableSize() == 0)
	{
		delete m_readBuf;
		delete m_writeBuf;
		delete m_request;
		delete m_response;
		m_evLoop->destoryChannel(m_channel);
	}

	Debug("连接断开,释放资源,conn_name: %s", m_name);
}

int TcpConnection::processRead(void* arg)
{
	TcpConnection* conn = (TcpConnection*)arg;
	// 接收数据
	int socket = conn->m_channel->getSocket();
	int count = conn->m_readBuf->socketRead(socket);
	Debug("接收到的http请求数据: %s", conn->m_readBuf->data());
	if (count > 0)
	{
		// 接收到了http请求,解析http请求
		bool flag = conn->m_request->parseHttpRequest(conn->m_readBuf, conn->m_response, conn->m_writeBuf, socket);
		if (!flag)
		{
			// 解析失败
			string err_msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
			conn->m_writeBuf->appendStr(err_msg);
		}
	}
	// 断开连接
	conn->m_evLoop->addTask(conn->m_channel, ELE_TYPE::DELETE);
	return 0;
}

int TcpConnection::processWrite(void* arg)
{
	Debug("开始发送数据了(基于写事件发送)");
	TcpConnection* conn = (TcpConnection*)arg;
	// 发送数据
	int count = conn->m_writeBuf->sendData(conn->m_channel->getSocket());
	if (count > 0)
	{
		// 判断数据是否被全部发送出去了
		if (conn->m_writeBuf->readableSize() == 0)
		{
			// 1 不再监测写事件 - 修改channel中保存的事件
			conn->m_channel->writeEventEnable(false);
			// 2 修改dispatcer监测的集合 - 添加任务节点
			conn->m_evLoop->addTask(conn->m_channel, ELE_TYPE::MODIFY);
			// 3 删除这个节点
			conn->m_evLoop->addTask(conn->m_channel, ELE_TYPE::DELETE);
		}
	}
	return count;
}

int TcpConnection::tcpConnDestory(void* arg)
{
	TcpConnection* conn = (TcpConnection*)arg;
	if (conn != nullptr)
	{
		delete conn;
	}
	return 0;
}
