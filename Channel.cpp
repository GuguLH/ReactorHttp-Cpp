#include "Channel.h"

Channel::Channel(int fd, FD_EVENT events, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryFunc, void* arg)
{
	m_fd = fd;
	m_events = (int)events;
	m_arg = arg;
	readCallback = readFunc;
	writeCallback = writeFunc;
	destoryCallback = destoryFunc;
}

void Channel::writeEventEnable(bool flag)
{
	if (flag)
	{
		// m_events |= (int)FD_EVENT::WRITE_EVENT;
		m_events |= static_cast<int>(FD_EVENT::WRITE_EVENT);
	}
	else
	{
		m_events &= ~static_cast<int>(FD_EVENT::WRITE_EVENT);
	}
}

bool Channel::isWriteEventEnable()
{
	return m_events & static_cast<int>(FD_EVENT::WRITE_EVENT);
}
