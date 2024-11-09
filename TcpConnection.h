#ifndef __TCP_CONNECTION_H__
#define __TCP_CONNECTION_H__

#include "Eventloop.h"
#include "Channel.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Log.h"


class TcpConnection
{
public:
	TcpConnection(int fd, Eventloop* evLoop);
	~TcpConnection();
private:
	static int processRead(void* arg);
	static int processWrite(void* arg);
	static int tcpConnDestory(void* arg);
private:
	Eventloop* m_evLoop;
	Channel* m_channel;
	Buffer* m_readBuf;
	Buffer* m_writeBuf;
	string m_name;
	HttpRequest* m_request;
	HttpResponse* m_response;
};

#endif