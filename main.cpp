#include "Connection.h"

SOCKET activeSocket;

int main()
{
	Connection c;

	// initializes the socket
	if (c.ItializeSocket() == false)
		return -1;
	else
	{
		while(true)
		{
			// prompts the user to select Server IP to connect
			if (c.SelectServer() == false)
				break;
			else
			{
				// binds the connection based on the given IP and Port and returns the active socket
				activeSocket = c.EstablishConnection();

				if (activeSocket == INVALID_SOCKET)
					break;

				// starts the client's activity
				Client* newClient = new Client(activeSocket);

				// checks if the client is still active and hangs the system for the given milliseconds
				while (newClient->_isActive == 1)
					Sleep(500);

				// breaks the loop if the client finished using the program
				// or deletes the client's activity in case of error
				if (newClient->_isActive == -1)
					break;
				else if(newClient->_isActive == 0)
					delete newClient;
			}
		}
	}

	// closes the socket connection
	c.CloseConnection();
	return 0;
}