#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>

using namespace std;

class Buffer
{
public:
	Buffer(int size);
	~Buffer();
	void resize(int size);
	inline int writeableSize()
	{
		return m_capacity - m_writePos;
	};
	inline int readableSize()
	{
		return m_writePos - m_readPos;
	};
	int appendStr(const char* data);
	int appendStr(const string data);
	int appendStr(const char* data, int size);
	int socketRead(int fd);
	char* findCRLF();
	int sendData(int socket);
	// 得到读数据的起始位置
	inline char* data()
	{
		return m_data + m_readPos;
	};
	inline int readPosIncr(int count)
	{
		m_readPos += count;
		return m_readPos;
	};
private:
	char* m_data = nullptr;
	int m_capacity = 0;
	int m_readPos = 0;
	int m_writePos = 0;
};

#endif