#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include "Client.h"


class Connection 
{

public:
	SOCKET EstablishConnection();
	bool ItializeSocket();
	bool SelectServer();
	void CloseConnection();
};