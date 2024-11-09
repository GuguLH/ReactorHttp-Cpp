#include "HttpRequest.h"

HttpRequest::HttpRequest()
{
	reset();
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::reset()
{
	m_curState = PARSE_STATE::REQ_LINE;
	m_method = m_url = m_version = string();
	m_reqHeaders.clear();
}

void HttpRequest::addHeader(const string key, const string value)
{
	if (key.empty() || value.empty())
	{
		return;
	}
	m_reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::getHeader(const string key)
{
	auto item = m_reqHeaders.find(key);
	if (item != m_reqHeaders.end())
	{
		return item->second;
	}
	return string();
}

bool HttpRequest::parseRequestLine(Buffer* readBuf)
{
	// 读出请求行,保存字符串结束地址
	char* end = readBuf->findCRLF();
	// 保存字符串起始地址
	char* start = readBuf->data();
	// 请求行总长度
	int lineSize = end - start;

	if (lineSize)
	{
		// 请求的方式
		auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", methodFunc);
		auto urlFunc = bind(&HttpRequest::setUrl, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", urlFunc);
		auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);
		splitRequestLine(start, end, NULL, versionFunc);

		// 为解析请求头做准备
		readBuf->readPosIncr(lineSize + 2);
		// 修改状态
		setState(PARSE_STATE::REQ_HEADERS);
		return true;
	}

	return false;
}

bool HttpRequest::parseRequestHeader(Buffer* readBuf)
{
	char* end = readBuf->findCRLF();
	if (end != nullptr)
	{
		char* start = readBuf->data();
		int lineSize = end - start;
		// 基于: 搜索字符
		char* middle = static_cast<char*>(memmem(start, lineSize, ": ", 2));
		if (middle != nullptr)
		{
			int keyLen = middle - start;
			int valLen = end - middle - 2;
			if (keyLen > 0 && valLen > 0)
			{
				string key(start, keyLen);
				string val(middle + 2, valLen);
				addHeader(key, val);
			}

			// 移动数据位置
			readBuf->readPosIncr(lineSize + 2);
		}
		else
		{
			// 请求头解析完毕,跳过空行
			readBuf->readPosIncr(2);
			// 修改解析状态
			setState(PARSE_STATE::REQ_DONE);
		}
		return true;
	}
	return false;
}

bool HttpRequest::parseHttpRequest(Buffer* readBuf, HttpResponse* resp, Buffer* writeBuf, int sockfd)
{
	bool flag = true;
	while (m_curState != PARSE_STATE::REQ_DONE)
	{
		switch (m_curState)
		{
		case PARSE_STATE::REQ_LINE:
			flag = parseRequestLine(readBuf);
			break;
		case PARSE_STATE::REQ_HEADERS:
			flag = parseRequestHeader(readBuf);
			break;
		case PARSE_STATE::REQ_BODY:
			break;
		default:
			break;
		}
		if (!flag)
		{
			return flag;
		}
		if (m_curState == PARSE_STATE::REQ_DONE)
		{
			// 服务器回复数据
			// 1 根据解析出的原始数据,对客户端的请求作出处理
			processHttpRequest(resp);
			// 2 组织响应数据并发送给客户端
			resp->prepareMsg(writeBuf, sockfd);
		}
	}
	setState(PARSE_STATE::REQ_LINE);
	return flag;
}

bool HttpRequest::processHttpRequest(HttpResponse* resp)
{
	// GET /abc.jpg
	if (strcasecmp(m_method.c_str(), "GET") != 0)
	{
		return false;
	}
	// 处理特殊字符
	m_url = decodeStr(m_url);
	const char* file = NULL;
	if (strcmp(m_url.c_str(), "/") == 0)
	{
		file = "./";
	}
	else
	{
		file = m_url.c_str() + 1;
	}

	int ret = 0;
	struct stat st;
	ret = stat(file, &st);
	if (ret == -1)
	{
		// 文件不存在,返回404
		resp->setFileName("404.html");
		resp->setStatusCode(RESPONSE_CODE::NOT_FOUND);
		// 响应头
		resp->addHeader("Content-type", getFileType(".html"));
		// auto func = bind(&HttpRequest::sendFile, this, placeholders::_1, placeholders::_2, placeholders::_3);
		resp->sendDataFunc = sendFile;

		return true;
	}

	resp->setFileName(file);
	resp->setStatusCode(RESPONSE_CODE::OK);
	if (S_ISDIR(st.st_mode))
	{
		// 把目录内容发送给客户端
		// 响应头
		resp->addHeader("Content-type", getFileType(".html"));
		resp->sendDataFunc = sendDir;
		return true;
	}
	else
	{
		// 把文件发送给客户端
		// 响应头
		resp->addHeader("Content-type", getFileType(file));
		resp->addHeader("Content-length", to_string(st.st_size));
		resp->sendDataFunc = sendFile;
		return true;
	}
	return false;
}

int HttpRequest::hexToDec(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	return 0;
}

string HttpRequest::decodeStr(string msg)
{
	string str = string();
	const char* from = msg.c_str();
	for (; *from != '\0'; from++)
	{
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));
			from += 2;
		}
		else
		{
			str.append(1, *from);
		}
	}
	str.append(1, '\0');
	return str;
}

const string HttpRequest::getFileType(const string name)
{
	// a.jpg a.mp4 a.html
	// 自右向左查找‘.’字符, 如不存在返回NULL
	const char* dot = strrchr(name.c_str(), '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8"; // 纯文本
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

void HttpRequest::sendDir(const string dirName, Buffer* sendBuf, int cfd)
{
	char buf[4096] = { 0 };
	sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName.c_str());

	struct dirent** namelist;
	int num = scandir(dirName.c_str(), &namelist, NULL, alphasort);
	for (int i = 0; i < num; i++)
	{
		char* name = namelist[i]->d_name;
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", dirName.c_str(), name);
		stat(subPath, &st);
		if (S_ISDIR(st.st_mode))
		{
			// a标签 <a href="">name</a>
			sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
		}
		else
		{
			sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
		}
		sendBuf->appendStr(buf);

		sendBuf->sendData(cfd);

		memset(buf, 0, sizeof(buf));
		free(namelist[i]);
	}
	sprintf(buf, "</table></body></html>");
	sendBuf->appendStr(buf);

	sendBuf->sendData(cfd);

	free(namelist);
}

void HttpRequest::sendFile(const string fileName, Buffer* sendBuf, int cfd)
{
	int fd = open(fileName.c_str(), O_RDONLY);
	assert(fd > 0);

	while (1)
	{
		char buf[1024] = { 0 };
		int len = read(fd, buf, sizeof(buf));
		if (len > 0)
		{
			// send(cfd, buf, len, 0);
			sendBuf->appendStr(buf, len);

			sendBuf->sendData(cfd);

		}
		else if (len == 0)
		{
			Debug("Send file succeed!\n");
			break;
		}
		else
		{
			Debug("Send file failed!\n");
			close(fd);
		}
	}
	close(fd);
}

char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback)
{
	char* space = const_cast<char*>(end);
	if (sub != nullptr)
	{
		space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
		assert(space != nullptr);
	}

	int length = space - start;
	callback(string(start, length));
	return space + 1;
}
