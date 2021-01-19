#include "Connection.h"
#include <iostream>
#include <ostream>
#include <sstream>




void Client::StaticUserRequests(LPVOID thisClient)
{
	Client* userRequest = (Client*)thisClient;
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
	if (threadMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		_isActive = 0;
		return;
	}

	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticDataTransfer, this, NULL, NULL);
	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StaticReceive, this, NULL, NULL);
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
		userinput == "8" || userinput == "9")
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
		if (!Entry)
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
		else if (_userinput == "p" || _userinput == "P")
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
		//char sendString[1024] = "H - Hello from client: ";
		std::string hello = "H - Hello from client: ";

		int len = send(_activeSocket, hello.c_str(), sizeof(hello), 0);
		if (len == SOCKET_ERROR)
		{
			std::cout << "len socket error" << std::endl;
			_isActive = 0;
		}
		Sleep(1000);
		ReceiveCommand(_commandType);
	}
	else if (_commandType == 2) // requests the most updated list
	{
		std::string request = "L - Requesting the most updated list. \n ";

		//char sendString[32] = "L - Requesting the most updated list. \n";
		int len = send(_activeSocket, request.c_str(), sizeof(request), 0);

		if (len == SOCKET_ERROR)
		{
			std::cout << "len socket error: " << WSAGetLastError() << std::endl;
			_isActive = 0;
		}
		ReceiveCommand(_commandType);
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

				//char sendString[1024];

				//strcpy(sendString, select.c_str());

				if (select != "No song") {
					int len = send(_activeSocket, select.c_str(), sizeof(select), 0);

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
			Sleep(1000);
		}
		else
		{
			std::cout << "List is empty." << std::endl;
		}
		ReceiveCommand(_commandType);
		//Sleep(1000);
	}
	else if (_commandType == 4) {

		std::string request = "P - Requesting Audio File Stream. \n";
		//char sendString[1024] = "P - Requesting Audio File Stream. \n";
		int len = send(_activeSocket, request.c_str(), sizeof(request), 0);
		if (len == SOCKET_ERROR)
		{
			std::cout << "len socket error: " << WSAGetLastError() << std::endl;
			_isActive = 0;
		}

		ReceiveCommand(_commandType);
	}
	else
	{
		_commandType = 0;
	}


	ReleaseMutex(threadMutex);
}

// based on the user's chosen command 
// receiving actions take place here
void Client::ReceiveCommand(int command) {

	switch (command) {

	case 1:
	{
		int bytes = 0;
		char recvMSG[4096];
		//ZeroMemory(recvMSG, 1024);
		int length = recv(_activeSocket, recvMSG, sizeof(recvMSG) + 1, NULL);
		//receive_till_zero(_activeSocket, recvMSG, bytes);

		if (length <= 0)
		{
			std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
			//_isActive = 0;
			break;
			SendRequests(0);
		}

		std::cout << recvMSG << std::endl;
		recvMSG[0] = '\0';
		break;
	}
	case 2:
	{
		int bytes = 0;
		wchar_t newList[4096];
		ZeroMemory(newList, 4096);

		int length = recv(_activeSocket, (char*)newList, sizeof(newList), NULL);

		//std::wcout << newList << std::endl;

		std::wstring serialList = newList;

		std::vector<std::wstring> newVecList = WSplit(serialList, '|');

		int i = 0;

		if (storeList(newVecList)) {
			std::cout << "Success!" << std::endl;
		}
		else {
			std::cout << "Error in transfering file names" << std::endl;
			break;
		}

		if (length <= 0)
		{
			std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
			_isActive = 0;
			break;
		}

		newList[0] = '\0';
		break;
	}

	case 3:
	{
		std::cout << "Command 3 has started" << std::endl;

		char recvMSG[4096];
		ZeroMemory(recvMSG, 4096);
		int length = recv(_activeSocket, recvMSG, sizeof(recvMSG) + 16, NULL);

		memmove(recvMSG, recvMSG + 1, sizeof(recvMSG));

		std::cout << recvMSG << std::endl;

		if (length <= 0)
		{
			std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
			_isActive = 0;
			break;
			SendRequests(0);
		}
		else {
			int i = 0;
			std::string trackInfo = recvMSG;

			std::vector<std::string> songDataList = Split(trackInfo, '|');
			do {
				std::cout << songDataList[i] << std::endl;
				i++;
			} while (i < songDataList.size());

			chunkSize = strtol(songDataList[0].c_str(),0,0);
			sampleRate = strtol(songDataList[1].c_str(), 0, 0);
			dataSize = strtol(songDataList[2].c_str(), 0, 0);
			channels = atoi(songDataList[3].c_str());
			bitsPerSample = atoi(songDataList[4].c_str());

			std::cout << chunkSize << " | "<< sampleRate << " | " << dataSize << " | " << channels << " | " << bitsPerSample << std::endl;

		}

		recvMSG[0] = '\0';
		DefineMenu(2);
		break;
	}

	case 4:
	{

		std::cout << "Play command started" << std::endl;
			char streamIn[4096];
			ZeroMemory(streamIn, 4096);
			int length = recv(_activeSocket, streamIn, sizeof(streamIn), NULL);
			if (length <= 0)
			{
				std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
				//_isActive = 0;
				break;
				//SendRequests(0);
			}

			bool result = InitializePrimary();

			if (!result) {
				std::cout << "Unable to initialise Primary Buffer" << std::endl;

				break;
			}

			std::cout << "Primary buffer initialization success!" << std::endl;

			result = InitializeSecondary();


			if (!result) {
				std::cout << "Unable to initialise Secondary Buffer" << std::endl;

				break;
			}

			BYTE* streamBytes = new BYTE[sizeof(streamIn) - 1];
			std::memcpy(streamBytes, streamIn, sizeof(streamIn) - 1);
			result = PlayWaveFile(streamBytes);

			if (result == true) {
				std::cout << "Playing song... " << std::endl;
			}
			else {

				std::cout << "Could not play song." << std::endl;
				break;
			}

			//std::cout << "Streaming has started!" << std::endl;
			//break;
	}


	}

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
bool Client::ReceiveData(char* data, int totalbytes)
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
std::vector<std::string> Client::Split(std::string stringToSplit, char delimeter)
{
	std::stringstream ss(stringToSplit);
	std::string item;
	std::vector<std::string> splittedStrings;
	while (std::getline(ss, item, delimeter))
	{
		splittedStrings.push_back(item);
	}
	return splittedStrings;
}

// It is used to split the string on specified delimeter
std::vector<std::wstring> Client::WSplit(std::wstring stringToSplit, wchar_t delimeter)
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


bool Client::storeList(std::vector<std::wstring> list) {


	for (int i = 0; i < list.size(); i++)
	{
		std::wcout << "[" << i << "] " << list[i] << std::endl;

		_storedList.push_back(list[i]);
		_totalCount += i;
	}

	if (&_storedList[0] != nullptr) {
		return true;
	}
	else {
		return false;
	}
}

void Client::getSongData() {

	int length = 1024;
	std::cout << "Waiting message..." << std::endl;

	do {
		char recvMSG[1024];
		ZeroMemory(recvMSG, 1024);
		int rec = recv(_activeSocket, recvMSG, 1024, NULL);
		if (rec <= 0)
		{
			std::cout << "\nReceive has failed. Error that occured: " << WSAGetLastError() << std::endl;
			_isActive = 0;
			break;
		}
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
				//std::cout << "You chose track: " << temp << std::endl;
			}
		}

		return select;
	}
	else {
		std::cout << "No such song found" << std::endl;
		return "No song";
	}
}

bool Client::InitializePrimary() {

	WAVEFORMATEXTENSIBLE wfx = { 0 };
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;

	// Initialize the direct sound interface pointer for the default sound device.
	result = DirectSoundCreate8(NULL, &directSound, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
	// We use the handle of the desktop window since we are a console application.  If you do write a 
	// graphical application, you should use the HWnd of the graphical application. 
	result = directSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the primary buffer description.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Initialize the direct sound interface pointer for the default sound device.
	result = directSound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	// Really, we should set this up from the wave file format loaded from the file.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = sampleRate;

	//44100;
	waveFormat.wBitsPerSample = bitsPerSample;

	//16;
	waveFormat.nChannels = channels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = chunkSize;

	// Set the primary buffer to be the wave format specified.
	result = primaryBuffer->SetFormat(&waveFormat);
	if (FAILED(result))
	{
		return false;
	}

	std::cout << "Primary Buffer Initialization success." << std::endl;

	return 0;
}

bool Client::InitializeSecondary() {
	WAVEFORMATEXTENSIBLE wfx = { 0 };
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;

	DWORD chunkSize;
	DWORD chunkPosition;
	DWORD filetype;
	HRESULT hr = S_OK;


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
	bufferDesc.dwBufferBytes = dataBufferSize;
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

	result = primaryBuffer->SetCurrentPosition(0);

	std::cout << "Playback being set at the beggining of the buffer." << std::endl;
	//result = secondaryBuffer->SetCurrentPosition(0);
	if (FAILED(result))
	{
		return false;
	}

	// Set volume of the buffer to 100%.
	result = primaryBuffer->SetVolume(DSBVOLUME_MAX);

	std::cout << "Buffer volume being set to max" << std::endl;
	//result = secondaryBuffer->SetVolume(DSBVOLUME_MAX);
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

	result = primaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&directSoundNotify);

	std::cout << "Event notification being created." << std::endl;
	//result = secondaryBuffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&directSoundNotify);
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

	result = primaryBuffer->Lock(0, dataBufferSize, (void**)&bufferPtr1, (DWORD*)&bufferSize1, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);

	std::cout << "First part of buffer is being locked" << std::endl;
	//result = secondaryBuffer->Lock(0, dataBufferSize, (void**)&bufferPtr1, (DWORD*)&bufferSize1, (void**)&bufferPtr2, (DWORD*)&bufferSize2, 0);
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


	result = primaryBuffer->Unlock((void*)bufferPtr1, bufferSize1, (void*)bufferPtr2, bufferSize2);

	std::cout << "Buffer is being unlocked." << std::endl;
	//result = secondaryBuffer->Unlock((void*)bufferPtr1, bufferSize1, (void*)bufferPtr2, bufferSize2);
	if (FAILED(result))
	{
		return false;
	}

	// Play the contents of the secondary sound buffer. If you want play to go back to the start of the buffer
	// again, set the last parameter to DSBPLAY_LOOPING instead of 0.  If play is already in progress, then 
	// play will just continue. 

	result = primaryBuffer->Play(0, 0, 0);

	std::cout << "Playing song." << std::endl;
	//result = secondaryBuffer->Play(0, 0, 0);
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