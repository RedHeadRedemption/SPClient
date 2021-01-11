#include "Connection.h"
#include <iostream>
#include <ostream>
#include <sstream>


void Client::StaticReceive(LPVOID thisClient)
{
	Client * clientReceive = (Client*)thisClient;
	clientReceive->Receive();
}

void Client::StaticUserRequests(LPVOID thisClient)
{
	Client * userRequest = (Client*)thisClient;
	userRequest->UserRequests();
}


// main constructor
Client::Client(SOCKET clientSocket) 
{
	_isActive = 1;
	_activeSocket = clientSocket;
	_checkTrack = "";
	_selectedTrack = L"";
	_commandType = 0;
	_totalCount = 0;

	threadMutex = CreateMutex(NULL, FALSE, NULL);
	if (threadMutex== NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		_isActive = 0;
		return;
	}

	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticDataTransfer, this, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticReceive, this, NULL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticUserRequests, this, NULL, NULL);
	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticClientThread, this, NULL, NULL);
}

// de-constructor
Client::~Client(void)
{
	if (_activeSocket == INVALID_SOCKET)
		closesocket(_activeSocket);
}


// displays the menu according to the user's choices
void Client::DefineMenu(int choice)
{
	if (choice == 1)
	{
		std::cout << "\nPress 'H' to send a Hello Message!" << std::endl;
		std::cout << "Press 'L' to download the Latest track List." << std::endl;
		std::cout << "Press 'V' to view the current list" << std::endl;
		std::cout << "Press 'N' to select the number of a corresponding track for playback." << std::endl;
		std::cout << "Press 'Q' to quit the program." << std::endl;
	}
	else if (choice == 2)
	{
		//getSongData();
		//int songData = recv(_activeSocket, recvMSG, 1024, NULL);
		//std::cout << "Average Bytes Per Second:" << std::endl;


		std::cout << "Press 'P' to play the selected track." << std::endl;
		std::cout << "Press 'D' to pause the playback or 'P' to resume." << std::endl;
		std::cout << "Press 'S' to stop the playback." << std::endl;
		std::cout << "Press 'Q' to quit the program.\n" << std::endl;
	}
	else
	{
		choice = 1;
	}

}

// checks if the user have chosen the string of a 
// number in order to determine what track chose to play
bool Client::CheckForNumberInput(std::string input)
{
	std::string userinput = input;

	if (userinput == "0" || userinput == "1" ||
		userinput == "2" || userinput == "3" ||
		userinput == "4" || userinput == "5" ||
		userinput == "6" || userinput == "7" ||
		userinput == "8" || userinput == "9" )
	{
		return true;
	}

	return false;
}


// main entry of the program
// runs on a separate thread in
// order to accept user input at any 
// given point while running the program
void Client::UserRequests()
{
	bool Entry = true;

	while (true)
	{
		if(!Entry)
			std::cout << "What do you want to do? Press 'M' for Menu: ";
		
		std::getline(std::cin, _userinput);

		if (_userinput == "m" || _userinput == "M")
		{
			if (_commandType == 3)
				DefineMenu(2);
			else
				SendRequests(0);
		}
		else if (_userinput == "h" || _userinput == "H")
		{
			SendRequests(1);
		}
		else if (_userinput == "l" || _userinput == "L")
		{
			SendRequests(2);
		}
		else if (_userinput == "v" || _userinput == "V")
		{
			ViewStoredList();
		}
		else if (CheckForNumberInput(_userinput))
		{
			_checkTrack = _userinput;
			SendRequests(3);
		}
		else if (_userinput == "q" || _userinput == "Q")
		{
			std::cout << "Closing..." << std::endl;
			_isActive = 0;
			break;
		}
		else if (_userinput == "n" || _userinput == "N")
		{
			SendRequests(4);
		}
		else
		{

			if (Entry)
			{
				SendRequests(0);
				Entry = false;
			}
			else
				std::cout << "SORRY, THERE ARE NO OTHER OPTIONS FOR THIS PROGRAM!" << std::endl;
			
		}

		std::cout << "\n" << std::endl;
	
	}
}

// 
//void Client::deserialize(std::vector<std::string> &restore, char* buffer, int total_count)
//{
//	int temp1 = 0;
//	int temp2 = 0;
//	int lastNull = 0;
//	bool isNum;
//
//	//std::cout << "BUFFER " << " is: " << buffer << std::endl;
//
//	while (lastNull != total_count)
//	{
//		//std::cout << "LAST NULL " << " is: " << lastNull << std::endl;
//		for (int i = lastNull; i < total_count; i++) {
//			const char *begin = &buffer[i];
//			int size = 0;
//			while (buffer[i++])
//			{
//				if (buffer[i] == 0)
//				{
//					temp1 = total_count - i;
//					temp2 = (total_count - temp1);
//					lastNull = temp2 + 1;
//					//std::cout << "LAST NULL " << " is: " << lastNull << std::endl;
//					size += 1;
//					break;
//				}
//				else
//				{
//					size += 1;
//				}
//			}
//			restore.push_back(std::string(begin, size));
//		}
//	}
//}

// determines the user's command and 
// takes the corresponding action
void Client::SendRequests(int commandType)
{
	DWORD sendRequests = WaitForSingleObject(threadMutex, INFINITE);

	_commandType = commandType;

	//std::cout << "TYPE: " << _commandType << std::endl;
	if (_commandType == 0)
		DefineMenu(1);
	else if (_commandType == 1) // sends a hello msg to the server; for testing purposes
	{
		char sendString[1024] = "H - Hello from client: ";

		int len = send(_activeSocket, sendString, sizeof(sendString), 0);
		if (len == SOCKET_ERROR)
		{
			std::cout << "len socket error" << std::endl;
			_isActive = 0;
		}
	}
	else if (_commandType == 2) // requests the most updated list
	{
		char sendString[1024] = "L - Requesting the most updated list. \n";
		int len = send(_activeSocket, sendString, sizeof(sendString), 0);

		if (len == SOCKET_ERROR)
		{
			std::cout << "len socket error: " << WSAGetLastError() << std::endl;
			_isActive = 0;
		}
	}
	// determines if the user chose any number (of a track) from the list to start streaming
	else if (_commandType == 3)
	{

		if (_storedList.empty() == false)
		{
			ViewStoredList();

			int userinput = std::stoi(_userinput);
			bool isIndexed = false;

			for (int i = 0; i < _storedList.size(); i++)
			{
				if (userinput == i)
				{
					isIndexed = true;
				}
			}

			if (isIndexed)
			{
				for (int i = 0; i < _totalCount; i++)
				{
					if (userinput == i)
					{
						_selectedTrack = _storedList[i];
						break;
					}
				}

				std::wcout << "You selected: " << _selectedTrack << std::endl;
				std::cout << "Preparing it for playback..." << std::endl;

				//std::wcout << "You selected: " << _selectedTrack << std::endl;
				//std::cout << "Preparing it for playback..." << std::endl;

				std::string select = selectSong(_userinput) + " - User Requests Song Number: " + selectSong(_userinput);

				char sendString[1024];

				strcpy(sendString, select.c_str());

				if (select != "No song") {
					int len = send(_activeSocket, sendString, sizeof(sendString), 0);

					//std::cout << select << std::endl;
					if (len == SOCKET_ERROR)
					{
						std::cout << "len socket error: " << WSAGetLastError() << std::endl;
						_isActive = 0;
					}
				}
			}
			else
			{
				std::cout << "Wrong index!" << std::endl;
			}
			Sleep(5000);
		}
		else
		{
			std::cout << "List is empty." << std::endl;
		}

		//Sleep(1000);
	}
	else if (_commandType == 4) {
	char sendString[1024] = "N - Requesting Audio File Stream. \n";
	int len = send(_activeSocket, sendString, sizeof(sendString), 0);
	if (len == SOCKET_ERROR)
	 {
		std::cout << "len socket error: " << WSAGetLastError() << std::endl;
		_isActive = 0;
	 }
	}
	else
	{
		_commandType = 0;
	}
	

	ReleaseMutex(threadMutex);
}


// fetches the current list that is temporarily stored.
// NO FILES WILL BE STORED OR PLAYED LOCALLY ON THE CLIENT
void Client::ViewStoredList()
{
	if (_storedList.empty())
	{
		std::cout << "\nThe list is empty!\n" << std::endl;
		
	}
	else
	{
		for (int i = 0; i < _storedList.size(); i++)
		{
			std::wcout << "[" << i << "] " << _storedList[i] << std::endl;
			_totalCount += i;
		}
	}
}

// received data by checking if all bytes have been sent
bool Client::ReceiveData(char * data, int totalbytes)
{
	int bytesreceived = 0; 
	while (bytesreceived < totalbytes)
	{
		int ReceiveCheck = recv(_activeSocket, data + bytesreceived, totalbytes - bytesreceived, NULL);
		if (ReceiveCheck == SOCKET_ERROR) 
			return false;
		else if (ReceiveCheck == -1) {
			std::cout << errno << std::endl;
		}
		bytesreceived += ReceiveCheck;
	}
	return true; 
}


// It is used to split the string on specified delimeter
std::vector<std::wstring> Client::Split(std::wstring stringToSplit, wchar_t delimeter)
{
	std::wstringstream ss(stringToSplit);
	std::wstring item;
	std::vector<std::wstring> splittedStrings;
	while (std::getline(ss, item, delimeter))
	{
		splittedStrings.push_back(item);
	}
	return splittedStrings;
}


// based on the user's chosen command 
// receiving actions take place here
void Client::Receive()
{
	while (_isActive == 1)
	{

		try
		{

			if (_commandType == 1)
			{
				char recvMSG[1024];
				ZeroMemory(recvMSG, 1024);
				int length = recv(_activeSocket, recvMSG, 1024, NULL);
				if (length <= 0)
				{
					std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
					//_isActive = 0;
					break;
					//SendRequests(0);
				}
				recvMSG[0] = '\0';
				std::cout << recvMSG << std::endl;

			}

			if (_commandType == 2)
			{
				int bytes = 0;
				wchar_t newList[46];
				ZeroMemory(newList, 46);
				int length = receive_till_zero(_activeSocket, newList, bytes);

				if (length <= 0)
				{
					std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
					_isActive = 0;
					break;
				}

				std::wstring serialList = newList;

				std::vector<std::wstring> newVecList = Split(serialList, '|');

				std::cout << std::endl;

				for (int i = 0; i < newVecList.size(); i++)
				{
					std::wcout << "[" << i << "] " << newVecList[i] << std::endl;
					_storedList.push_back(newVecList[i]);
					_totalCount += i;
				}

				std::wcout << _storedList[0] << std::endl;
			}
				
			if (_commandType == 3)
			{
				std::cout << "Command 3 has started" << std::endl;

				
				int bytes = 0;
				wchar_t recvMSG[1024];
				ZeroMemory(recvMSG, 1024);
				int length = receive_till_zero(_activeSocket, recvMSG, bytes);

				//std::cout << length << std::endl;
				if (length <= 0)
				/*if(ReceiveData((char*)recvMSG, sizeof(recvMSG)) == false)*/
				{
					std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
					_isActive = 0;
					break;
					SendRequests(0);
				}
				else {
					std::cout << "Warning" << std::endl;
				}

			std::wstring trackInfo = recvMSG;
			recvMSG[0] = '\0';
			std::wcout << trackInfo << std::endl;

				DefineMenu(2);

			}
			if (_commandType == 4) {

				char recvMSG[1024];
				int length = recv(_activeSocket, recvMSG, 1024, NULL);
				if (length <= 0)
				{
					std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
					//_isActive = 0;
					break;
					//SendRequests(0);
				}
				recvMSG[0] = '\0';
				std::cout << "Streaming has started!" << std::endl;

			}
			
		}
		catch (...)
		{
			std::cout << "\nError was Caught: " << WSAGetLastError() << std::endl;
		}
	}
	
}

void Client::getSongData() {

	int length = 1024;
	std::cout << "Waiting message..." << std::endl;

	do {
		char recvMSG[1024];
		ZeroMemory(recvMSG,1024);
		int rec = recv(_activeSocket, recvMSG, 1024, NULL);
		if (rec <= 0)
		{
			std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
			_isActive = 0;
			break;
		}
		recvMSG[0] = '\0';
		std::cout << "Message recieved:" << recvMSG[0] << std::endl;
		length--;
	} while (length > 1);

}

std::string Client::selectSong(std::string input) {

	//std::string userInput = input;
	std::string select;
	int selection;
	if (CheckForNumberInput(input))
	{

		for (int i = 0; i < _storedList.size(); i++)
		{
			int temp = std::stoi(input);
			if (temp == i)
			{
				_selectedTrack = _storedList[i];
				selection = temp + 1;
				select = std::to_string(selection);
				//std::cout << "You chose track: " << temp << std::endl
			}
		}

		return select;
	}
	else {
		std::cout << "No such song found" << std::endl;
		return "No song";
	}
}


bool Client::InitializePlayer(BYTE* recvbuffer) {

	WAVEFORMATEXTENSIBLE wfx = { 0 };
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
	
	// Set the wave format of the secondary buffer that this wave file will be loaded onto.
	// The value of wfx.Format.nAvgBytesPerSec will be very useful to you since it gives you
	// an approximate value for how many bytes it takes to hold one second of audio data.
	waveFormat.wFormatTag = wfx.Format.wFormatTag;
	waveFormat.nSamplesPerSec = wfx.Format.nSamplesPerSec;
	waveFormat.wBitsPerSample = wfx.Format.wBitsPerSample;
	waveFormat.nChannels = wfx.Format.nChannels;
	waveFormat.nBlockAlign = wfx.Format.nBlockAlign;
	waveFormat.nAvgBytesPerSec = wfx.Format.nAvgBytesPerSec;
	waveFormat.cbSize = 0;

	// Set the buffer description of the secondary sound buffer that the wave file will be loaded onto. In
	// this example, we setup a buffer the same size as that of the audio data.  For the assignment, your
	// secondary buffer should only be large enough to hold approximately four seconds of data. 
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);

	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
	bufferDesc.dwBufferBytes = waveFormat.nAvgBytesPerSec * 4;
	//dataBufferSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Create a temporary sound buffer with the specific buffer settings.
	result = directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Test the buffer format against the direct sound 8 interface and create the secondary buffer.
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&secondaryBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the temporary buffer.
	tempBuffer->Release();
	tempBuffer = nullptr;

	return true;
}

int Client::receive_till_zero(SOCKET sock, wchar_t* tmpbuf, int& numbytes) {
  int i = 0;
  std::wstring Converter;
  do {
	std::cout << "numbytes: " << numbytes << std::endl;
	// Check if we have a complete message
	for (; i < numbytes; i++) {
	  if (tmpbuf[i] == '\0') {
		// \0 indicate end of message! so we are done
		return i + 1; // return length of message
	  }
	}
	long n = recv(sock, (char*)tmpbuf + numbytes, sizeof(tmpbuf), 0);
	Converter = tmpbuf;
	std::wcout << "Server >> " << Converter << std::endl;

	if (n == SOCKET_ERROR) {
	  std::cerr << "Error in recv(). Quitting...." << std::endl;
	  return 0;
	  break;
	}

	else if (n == 0) {
	  std::cout << "Server disconnected..." << std::endl;
	  return 0;
	  break;
	}

	//numbytes, 0);
	if (n == -1) {
	  return -1; // operation failed!
	}
	numbytes += n;
  } while (true);
}

bool Client::PlayWaveFile(BYTE* recvbuffer)
{
	dataBuffer = recvbuffer;

	HRESULT result;
	unsigned char* bufferPtr1;
	unsigned long   bufferSize1;
	unsigned char* bufferPtr2;
	unsigned long   bufferSize2;
	BYTE* dataBufferPtr = dataBuffer;
	DWORD soundBytesOutput = 0;
	bool fillFirstHalf = true;
	LPDIRECTSOUNDNOTIFY8 directSoundNotify;
	DSBPOSITIONNOTIFY positionNotify[2];

	// Set position of playback at the beginning of the sound buffer.
	result = secondaryBuffer->SetCurrentPosition(0);
	if (FAILED(result))
	{
		return false;
	}

	// Set volume of the buffer to 100%.
	result = secondaryBuffer->SetVolume(DSBVOLUME_MAX);
	if (FAILED(result))
	{
		return false;
	}

	// Create an event for notification that playing has stopped.  This is only useful
	// when your audio file fits in the entire secondary buffer (as in this example).  
	// For the assignment, you are going to need notifications when the playback has reached the 
	// first quarter of the buffer or the third quarter of the buffer so that you know when 
	// you should copy more data into the secondary buffer. 
	HANDLE playEventHandles[1];
	playEventHandles[0] = CreateEvent(NULL, FALSE, FALSE, NULL);

	result = secondaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&directSoundNotify);
	if (FAILED(result))
	{
		return false;
	}
	// This notification is used to indicate that we have finished playing the buffer of audio. In
	// the assignment, you will need two different notifications as mentioned above. 
	positionNotify[0].dwOffset = DSBPN_OFFSETSTOP;
	positionNotify[0].hEventNotify = playEventHandles[0];
	directSoundNotify->SetNotificationPositions(1, positionNotify);
	directSoundNotify->Release();

	// Now we can fill our secondary buffer and play it.  In the assignment, you will not be able to fill
	// the buffer all at once since the secondary buffer will not be large enough.  Instead, you will need to
	// loop through the data that you have retrieved from the server, filling different sections of the 
	// secondary buffer as you receive notifications.

	// Lock the first part of the secondary buffer to write wave data into it. In this case, we lock the entire
	// buffer, but for the assignment, you will only want to lock the half of the buffer that is not being played.
	// You will definately want to look up the methods for the IDIRECTSOUNDBUFFER8 interface to see what these
	// methods do and what the parameters are used for. 
	result = secondaryBuffer->Lock(0, dataBufferSize, (void**)&bufferPtr1, (DWORD*)&bufferSize1, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);
	if (FAILED(result))
	{
		return false;
	}
	// Copy the wave data into the buffer. If you need to insert some silence into the buffer, insert values of 0.
	memcpy(bufferPtr1, dataBuffer, bufferSize1);
	if (bufferPtr2 != NULL)
	{
		memcpy(bufferPtr2, dataBuffer, bufferSize2);
	}
	// Unlock the secondary buffer after the data has been written to it.
	result = secondaryBuffer->Unlock((void*)bufferPtr1, bufferSize1, (void*)bufferPtr2, bufferSize2);
	if (FAILED(result))
	{
		return false;
	}
	// Play the contents of the secondary sound buffer. If you want play to go back to the start of the buffer
	// again, set the last parameter to DSBPLAY_LOOPING instead of 0.  If play is already in progress, then 
	// play will just continue. 
	result = secondaryBuffer->Play(0, 0, 0);
	if (FAILED(result))
	{
		return false;
	}
	// Wait for notifications.  In this case, we only have one notification so we could use WaitForSingleObject,
	// but for the assignment you will need more than one notification, so you will need WaitForMultipleObjects
	result = WaitForMultipleObjects(1, playEventHandles, FALSE, INFINITE);
	// In this case, we have been notified that playback has finished so we can just finish. In the assignment,
	// you should use the appropriate notification to determine which part of the secondary buffer needs to be
	// filled and handle it accordingly.
	CloseHandle(playEventHandles[0]);
	return true;
}

