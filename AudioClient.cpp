#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <windows.h>
#include "stdio.h"
#include "ClientSession.h"

#pragma comment (lib, "ws2_32.lib")

HANDLE hStdin;
DWORD fdwSaveOldMode;
char ip[32];
int port = 5000;
SOCKET clientSocket = INVALID_SOCKET;

bool SetServer()
{
  if (ip[0] == 0)
  {
	printf("Server IP Address: ");
	scanf("%s", ip);
  }
  else
  {
	printf("Server IP Address: %s\n", ip);
	printf("Press 'C' to continue, 'S' to set server and 'Q' to exit.\n");
	char key[32];
	scanf("%s", &key);
	if (key[0] == 's' || key[0] == 'S')
	{
	  printf("Server IP Address: ");
	  scanf("%s", ip);
	}
	else if (key[0] == 'q' || key[0] == 'Q') return false;
  }
  return true;
}

bool Initialize()
{
  ip[0] = 0;
  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0)
  {
	printf("WSAStartup failed with error: %d\n", iResult);
	return false;
  }
  return true;
}

void Close()
{
  if (clientSocket != INVALID_SOCKET)
  {
	closesocket(clientSocket);
	clientSocket = INVALID_SOCKET;
  }
  WSACleanup();
}

int main()
{
  if (!Initialize()) return -1;

  while (true)
  {
	if (!SetServer()) break;

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(ip);
	sa.sin_port = htons(port);

	// create socket for connecting to server
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
	  printf("socket failed with error: %d\n", WSAGetLastError());
	  continue;
	}

	// Connect to server.
	if (connect(clientSocket, (SOCKADDR*)&sa, sizeof(sa)) == SOCKET_ERROR)
	{
	  closesocket(clientSocket);
	  clientSocket = INVALID_SOCKET;
	  printf("connect failed with error: %d\n\n", WSAGetLastError());
	  continue;
	}

	ClientSession* session = new ClientSession(clientSocket);

	while (session->connectionStatus == 1) Sleep(500);
	int alive = session->connectionStatus;
	delete session;
	if (alive == -1) break;
  }

  Close();
  return 0;
}

