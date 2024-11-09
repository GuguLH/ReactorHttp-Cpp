#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <string>
#include <map>
#include <functional>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Buffer.h"
#include "HttpResponse.h"
#include "Log.h"

using namespace std;

enum class PARSE_STATE :char
{
	REQ_LINE,
	REQ_HEADERS,
	REQ_BODY,
	REQ_DONE
};

class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();
	// 重置
	void reset();
	// 添加请求头
	void addHeader(const string key, const string value);
	// 根据key得到value
	string getHeader(const string key);
	// 解析请求行
	bool parseRequestLine(Buffer* readBuf);
	// 解析请求头
	bool parseRequestHeader(Buffer* readBuf);
	// 解析http请求协议
	bool parseHttpRequest(Buffer* readBuf, HttpResponse* resp, Buffer* writeBuf, int sockfd);
	// 处理http请求协议
	bool processHttpRequest(HttpResponse* resp);
	// 获取当前处理状态
	inline PARSE_STATE getState()
	{
		return m_curState;
	};
	inline void setState(PARSE_STATE state)
	{
		m_curState = state;
	};

private:
	int hexToDec(char c);
	string decodeStr(string from);
	const string getFileType(const string name);
	static void sendDir(const string dirName, Buffer* sendBuf, int cfd);
	static void sendFile(const string fileName, Buffer* sendBuf, int cfd);
	inline void setMethod(string method)
	{
		m_method = method;
	};
	inline void setUrl(string url)
	{
		m_url = url;
	};
	inline void setVersion(string version)
	{
		m_version = version;
	};
	char* splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback);

private:
	string m_method;
	string m_url;
	string m_version;
	map<string, string> m_reqHeaders;
	PARSE_STATE m_curState;
};

#endif