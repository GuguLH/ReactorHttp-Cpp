#include "SelectDispatcher.h"

SelectDispatcher::SelectDispatcher(Eventloop* evloop) :Dispatcher(evloop)
{
	FD_ZERO(&m_readSet);
	FD_ZERO(&m_writeSet);
	m_name = "Select";
}

SelectDispatcher::~SelectDispatcher()
{
}

int SelectDispatcher::add()
{
	if (m_channel->getSocket() >= m_maxSize)
	{
		return -1;
	}
	setFdSet();
	return 0;
}

int SelectDispatcher::remove()
{
	clearFdSet();
	// 通过channel释放对应的TcpConnection资源
	m_channel->destoryCallback(const_cast<void*>(m_channel->getArg()));
	return 0;
}

int SelectDispatcher::modify()
{
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::READ_EVENT))
	{
		FD_SET(m_channel->getSocket(), &m_readSet);
		FD_CLR(m_channel->getSocket(), &m_writeSet);
	}
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::WRITE_EVENT))
	{
		FD_SET(m_channel->getSocket(), &m_writeSet);
		FD_CLR(m_channel->getSocket(), &m_readSet);
	}
	return 0;
}

int SelectDispatcher::dispatch(int timeout)
{
	fd_set rd_tmp = m_readSet;
	fd_set wr_tmp = m_writeSet;
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	int count = select(m_maxSize, &rd_tmp, &wr_tmp, NULL, &tv);
	if (count == -1)
	{
		perror("Error select()");
		return -1;
	}
	for (int i = 0; i < m_maxSize; i++)
	{
		if (FD_ISSET(i, &rd_tmp))
		{
			// 处理读就绪
			m_evLoop->eventActive(i, static_cast<int>(FD_EVENT::READ_EVENT));
		}
		if (FD_ISSET(i, &wr_tmp))
		{
			// 处理写就绪
			m_evLoop->eventActive(i, static_cast<int>(FD_EVENT::WRITE_EVENT));
		}
	}
	return 0;
}

void SelectDispatcher::setFdSet()
{
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::READ_EVENT))
	{
		FD_SET(m_channel->getSocket(), &m_readSet);
	}
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::WRITE_EVENT))
	{
		FD_SET(m_channel->getSocket(), &m_writeSet);
	}
}

void SelectDispatcher::clearFdSet()
{
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::READ_EVENT))
	{
		FD_CLR(m_channel->getSocket(), &m_readSet);
	}
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::WRITE_EVENT))
	{
		FD_CLR(m_channel->getSocket(), &m_writeSet);
	}
}
