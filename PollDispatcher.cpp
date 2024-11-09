#include "PollDispatcher.h"

PollDispatcher::PollDispatcher(Eventloop* evloop) : Dispatcher(evloop)
{
	m_maxFd = 0;
	m_fds = new struct pollfd[m_maxNode];
	for (int i = 0; i < m_maxNode; i++)
	{
		m_fds[i].fd = -1;
		m_fds[i].events = 0;
		m_fds[i].revents = 0;
	}
	m_name = "Poll";
}

PollDispatcher::~PollDispatcher()
{
	delete[] m_fds;
}

int PollDispatcher::add()
{
	int events = 0;
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::READ_EVENT))
	{
		events |= POLLIN;
	}
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::WRITE_EVENT))
	{
		events |= POLLOUT;
	}

	int i = 0;
	for (i = 0; i < m_maxNode; i++)
	{
		if (m_fds[i].fd == -1)
		{
			m_fds[i].fd = m_channel->getSocket();
			m_fds[i].events = (short)events;
			m_maxFd = i > m_maxFd ? i : m_maxFd;
			break;
		}
	}
	if (i >= m_maxNode)
	{
		return -1;
	}
	return 0;
}

int PollDispatcher::remove()
{
	int i = 0;
	for (i = 0; i < m_maxNode; i++)
	{
		if (m_fds[i].fd == m_channel->getSocket())
		{
			m_fds[i].fd = -1;
			m_fds[i].events = 0;
			m_fds[i].revents = 0;
			break;
		}
	}
	// 通过channel释放对应的TcpConnection资源
	m_channel->destoryCallback(const_cast<void*>(m_channel->getArg()));
	if (i >= m_maxNode)
	{
		return -1;
	}
	return 0;
}

int PollDispatcher::modify()
{
	int events = 0;
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::READ_EVENT))
	{
		events |= POLLIN;
	}
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::WRITE_EVENT))
	{
		events |= POLLOUT;
	}

	int i = 0;
	for (i = 0; i < m_maxNode; i++)
	{
		if (m_fds[i].fd == m_channel->getSocket())
		{
			m_fds[i].events = (short)events;
			break;
		}
	}
	if (i >= m_maxNode)
	{
		return -1;
	}
	return 0;
}

int PollDispatcher::dispatch(int timeout)
{
	int count = poll(m_fds, m_maxFd + 1, timeout * 1000);
	if (count == -1)
	{
		perror("Error poll()");
		return -1;
	}
	for (int i = 0; i <= m_maxFd; i++)
	{
		if (m_fds[i].fd == -1)
		{
			continue;
		}
		if (m_fds[i].revents & POLLIN)
		{
			m_evLoop->eventActive(m_fds[i].fd, static_cast<int>(FD_EVENT::READ_EVENT));
		}
		if (m_fds[i].revents & POLLOUT)
		{
			m_evLoop->eventActive(m_fds[i].fd, static_cast<int>(FD_EVENT::WRITE_EVENT));
		}
	}
	return 0;
}
