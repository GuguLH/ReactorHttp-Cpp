#include "HttpResponse.h"

HttpResponse::HttpResponse()
{
	m_statusCode = RESPONSE_CODE::UNKNOWN;
	m_headers.clear();
	m_fileName = string();
	sendDataFunc = nullptr;
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::addHeader(const string key, const string value)
{
	if (key.empty() || value.empty())
	{
		return;
	}
	m_headers.insert(make_pair(key, value));
}

void HttpResponse::prepareMsg(Buffer* sendBuf, int socket)
{
	// 状态行
	char tmp[1024] = { 0 };
	int code = static_cast<int>(m_statusCode);
	sprintf(tmp, "HTTP/1.1 %d %s\r\n", code, m_info.at(code).c_str());
	sendBuf->appendStr(tmp);
	// 响应头
	for (auto it = m_headers.begin(); it != m_headers.end(); it++)
	{
		sprintf(tmp, "%s: %s\r\n", it->first.c_str(), it->second.c_str());
		sendBuf->appendStr(tmp);
	}
	// 空行
	sendBuf->appendStr("\r\n");
	
	sendBuf->sendData(socket);

	// 回复的数据
	sendDataFunc(m_fileName, sendBuf, socket);
}
