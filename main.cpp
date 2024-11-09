#include <iostream>

#include "TcpServer.h"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <port> <pathname>.\n", argv[0]);
	}

	int ret = 0;
	// 切换工作目录
	ret = chdir(argv[2]);
	if (ret == -1)
	{
		perror("Error chdir()");
		return -1;
	}

	unsigned short port = atoi(argv[1]);

	TcpServer* server = new TcpServer(port, 41);
	server->run();
}