#include "Buffer.h"

Buffer::Buffer(int size)
{
	m_capacity = size;
	m_data = (char*)malloc(size);
	bzero(m_data, size);
}

Buffer::~Buffer()
{
	if (m_data != nullptr)
	{
		free(m_data);
	}
}

void Buffer::resize(int size)
{
	// 1 内存够用-不需要扩容
	if (writeableSize() >= size)
	{
		return;
	}
	// 2 内存需要合并才够用-不需要扩容
	// 剩余可写的内存 + 已读的内存
	else if (m_readPos + writeableSize() >= size)
	{
		int readable = readableSize();
		memcpy(m_data, m_data + m_readPos, readable);
		m_readPos = 0;
		m_writePos = readable;
	}
	// 3 内存不够用-扩容
	else
	{
		void* tmp = realloc(m_data, m_capacity + size);
		if (tmp == nullptr)
		{
			return;
		}
		memset((char *)tmp + m_capacity, 0, size);
		// 更新数据
		m_data = static_cast<char*>(tmp);
		m_capacity += size;
	}
}

int Buffer::appendStr(const char* data)
{
	int size = strlen(data);
	int ret = appendStr(data, size);
	return ret;
}

int Buffer::appendStr(const string data)
{
	int ret = appendStr(data.c_str());
	return ret;
}

int Buffer::appendStr(const char* data, int size)
{
	if (data == nullptr || size <= 0)
	{
		return -1;
	}
	// 扩容
	resize(size);
	// 数据拷贝
	memcpy(m_data + m_writePos, data, size);
	m_writePos += size;
	return 0;
}

int Buffer::socketRead(int fd)
{
	struct iovec vec[2];
	int writeable = writeableSize();
	vec[0].iov_base = m_data + m_writePos;
	vec[0].iov_len = writeable;

	char* tmp_buf = (char*)malloc(40960);
	vec[1].iov_base = tmp_buf;
	vec[1].iov_len = 40960;
	int ret = readv(fd, vec, 2);
	if (ret == -1)
	{
		return -1;
	}
	else if (ret <= writeable)
	{
		m_writePos += ret;
	}
	else
	{
		m_writePos = m_capacity;
		appendStr(tmp_buf, ret - writeable);
	}
	free(tmp_buf);
	return ret;
}

char* Buffer::findCRLF()
{
	char* ptr = static_cast<char *>(memmem(m_data + m_readPos, readableSize(), "\r\n", 2));
	return ptr;
}

int Buffer::sendData(int socket)
{
	// 判断有无数据
	int readable = readableSize();
	if (readable > 0)
	{
		int count = send(socket, m_data + m_readPos, readable, MSG_NOSIGNAL);
		if (count)
		{
			m_readPos += count;
			usleep(1);
		}
		return count;
	}
	return 0;
}
