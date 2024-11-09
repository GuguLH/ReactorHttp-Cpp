#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include <string>
#include <map>
#include <functional>

#include "Buffer.h"

using namespace std;

enum class RESPONSE_CODE
{
	UNKNOWN,
	OK = 200,
	MOVED_PERMANENTLY = 301,
	MOVED_TEMPORARILY = 302,
	BAD_REQUEST = 400,
	NOT_FOUND = 404
};

class HttpResponse
{
public:
	HttpResponse();
	~HttpResponse();
	// 添加响应头
	void addHeader(const string key, const string value);
	// 组织http响应数据
	void prepareMsg(Buffer* sendBuf, int socket);
	function<void(const string, Buffer*, int)> sendDataFunc;
	inline void setFileName(string name)
	{
		m_fileName = name;
	};
	inline void setStatusCode(RESPONSE_CODE code)
	{
		m_statusCode = code;
	};
private:
	// 状态行
	RESPONSE_CODE m_statusCode;
	string m_fileName;
	// 响应头 - 键值对
	map<string, string> m_headers;
	// 定义状态码和描述对应关系
	const map<int, string> m_info = {
		{200, "OK"},
		{301, "MOVED_PERMANENTLY"},
		{302, "MOVED_TEMPORARILY"},
		{400, "BAD_REQUEST"},
		{404, "NOT_FOUND"},
	};
};

#endif