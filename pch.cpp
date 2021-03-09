// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。
bool TestBindPort(u_short port)
{
	bool f = false;
	//初始化WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) == 0)
	{
		//创建套接字  
		SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (slisten != INVALID_SOCKET)
		{
			//绑定IP和端口  
			sockaddr_in sin;
			sin.sin_family = AF_INET;
			sin.sin_port = htons(port);
			//sin.sin_addr.S_un.S_addr = INADDR_ANY;
			sin.sin_addr.s_addr = inet_addr("127.0.0.1");
			if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
			{
				f = false;
			}
			else {
				f = true;
			}
			closesocket(slisten);
		}
		WSACleanup();
	}
	return f;
}
std::string GetGuid()
{
	std::string strguid = "";
	char buffer[64] = { 0 };
	GUID guid;
	if (CoCreateGuid(&guid) == S_OK)
	{
		_snprintf_s(buffer, sizeof(buffer),
			"%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
		strguid = buffer;		
	}
	return strguid;
}