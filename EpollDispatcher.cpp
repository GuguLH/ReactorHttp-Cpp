#include "EpollDispatcher.h"

EpollDispatcher::EpollDispatcher(Eventloop* evloop) : Dispatcher(evloop)
{
	m_epfd = epoll_create(1);
	if (m_epfd == -1)
	{
		perror("Error epoll_create()");
		exit(EXIT_FAILURE);
	}
	m_events = new struct epoll_event[m_maxNode];
	m_name = "Epoll";
}

EpollDispatcher::~EpollDispatcher()
{
	close(m_epfd);
	delete[] m_events;
}

int EpollDispatcher::epollControl(int op)
{
	struct epoll_event ev;
	ev.data.fd = m_channel->getSocket();
	int events = 0;
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::READ_EVENT))
	{
		events |= EPOLLIN;
	}
	if (m_channel->getEvent() & static_cast<int>(FD_EVENT::WRITE_EVENT))
	{
		events |= EPOLLOUT;
	}
	ev.events = events;
	int ret = epoll_ctl(m_epfd, op, m_channel->getSocket(), &ev);
	return ret;
}

int EpollDispatcher::add()
{
	int ret = epollControl(EPOLL_CTL_ADD);
	if (ret == -1)
	{
		perror("Error epoll_add()");
		return -1;
	}
	return ret;
}

int EpollDispatcher::remove()
{
	int ret = epollControl(EPOLL_CTL_DEL);
	if (ret == -1)
	{
		perror("Error epoll_remove()");
		return -1;
	}
	// 通过channel释放对应的TcpConnection资源
	ret = m_channel->destoryCallback(const_cast<void*>(m_channel->getArg()));
	return ret;
}

int EpollDispatcher::modify()
{
	int ret = epollControl(EPOLL_CTL_MOD);
	if (ret == -1)
	{
		perror("Error epoll_modify()");
		return -1;
	}
	return ret;
}

int EpollDispatcher::dispatch(int timeout)
{
	int count = epoll_wait(m_epfd, m_events, m_maxNode, timeout * 1000);
	for (int i = 0; i < count; i++)
	{
		int events = m_events[i].events;
		int fd = m_events[i].data.fd;
		if ((events & EPOLLERR) || (events & EPOLLHUP))
		{
			// 对方断开了连接,删除fd
			// epoll_remove()
			continue;
		}
		if (events & EPOLLIN)
		{
			m_evLoop->eventActive(fd, static_cast<int>(FD_EVENT::READ_EVENT));
		}
		if (events & EPOLLOUT)
		{
			m_evLoop->eventActive(fd, static_cast<int>(FD_EVENT::WRITE_EVENT));
		}
	}
	return 0;
}
