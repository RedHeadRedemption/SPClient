#include "Connection.h"

SOCKET clientSocket;
int port = 5000;  // for a different port change this value
char ipAddress[100];

// a menu that prompts the user to select Server IP to connect
bool Connection::SelectServer()
{
	if (ipAddress[0] == 0)
	{
		printf("Welcome to Music Streaming!\n\nPlease select the IP Address of the Server: ");
		scanf("%s", ipAddress);
	}
	else
	{
		printf("Please select the IP Address of the Server: %s\n", ipAddress);
		printf("Press any key to try again, or press 'S' to try another IP Address or press 'Q' to quit.\n");

		char selection[100];
		scanf("%s", &selection);

		if (selection[0] == 'q' || selection[0] == 'Q')
			return false;	
		else if (selection[0] == 's' || selection[0] == 'S')
		{
			printf("Please select the IP Address of the Server: ");
			scanf("%s", ipAddress);
		}
			
	}
	return true;
}

// closes the socket connection
void Connection::CloseConnection()
{
	if (clientSocket == INVALID_SOCKET)
	{
		clientSocket = INVALID_SOCKET;
		closesocket(clientSocket);
	}
	
	WSACleanup();
}

// initializes the socket
bool Connection::ItializeSocket()
{
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 2);

	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "WinSock initialization has failed", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

// starts a connection based on the given IP and port and returns the active socket
SOCKET Connection::EstablishConnection()
{	
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr(ipAddress);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
 
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	if (connect(clientSocket, (SOCKADDR*)&addr, sizeofaddr) != 0)
	{
		MessageBoxA(NULL, "Unable to connect to a server", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
		
	return clientSocket;
}


