#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "ClientSession.h"
#include <mmreg.h>


void instantiateDataSender(void* param)
{
  ClientSession* session = (ClientSession*)param;
  session->sendingDataHandler();
}

void instantiateDataReceiver(void* param)
{
  ClientSession* session = (ClientSession*)param;
  session->receivingDataHandler();
}

void intantiateUi(void* param)
{
  ClientSession* session = (ClientSession*)param;
  session->uiHandler();
}

ClientSession::ClientSession(SOCKET clientSocket)
{
  connectionStatus = isAlive;
  _clientSocket = clientSocket;
  _songListNames = NULL;
  _songListIndex = 0;
  _songListSize = 0;
  _songNumber = 0;
  _menuState = 1; // MainMenu
  _receivedIndex = 0;
  _receivedDataSize = 0;
  _songSize = 0;
  _songIndex = 0;
  _sendStop = false;

  _request.starBites = 0x12345678;
  _request.requestCommand = Status;
  _request.dataSize = sizeof(DataHolder);
  _request.index = 0;
  _request.count = 0;
  _request.dataAmountSent = 0;
  _request.stopBits = 0x87654321;

  // create mutex to synthro
  _mutex = CreateMutex(NULL, FALSE, NULL);
  if (_mutex == NULL)
  {
	printf("CreateMutex error: %d\n", GetLastError());
	connectionStatus = isDeleted;
	return;
  }

  // create threads for keyboard, sending and receiving.
  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)instantiateDataSender, this, 0, NULL);
  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)instantiateDataReceiver, this, 0, NULL);
  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)intantiateUi, this, 0, NULL);
}

ClientSession::~ClientSession(void)
{
  if (_clientSocket != INVALID_SOCKET)
  {
	closesocket(_clientSocket);
  }
  if (_songListNames != NULL)
  {
	for (int i = 0; i < _songListSize; i++)
	{
	  if (_songListNames[i] != NULL) delete[] _songListNames[i];
	}
  }
}

void ClientSession::uiHandler()
{
  char cmd[32];
  while (true)
  {
	if (_menuState == 0) Sleep(1000); // None
	else if (_menuState == 1) // MainMenu
	{
	  printf("\n[L] get music list from server\n");
	  printf("[V] view music list\n");
	  printf("[number] play the music with number\n");
	  printf("[Q] exit\n");
	  printf("Your choice: ");
	  scanf("%s", &cmd);

	  if (cmd[0] == 'l' || cmd[0] == 'L')
	  {
		_setSendingBuffer(ListInfo); // ListInfo
		_menuState = 0;
		printf("\nDownloading the music list from server...\n\n");
	  }
	  else if (cmd[0] == 'v' || cmd[0] == 'V')
	  {
		if (_songListSize == 0)
		{
		  _setSendingBuffer(ListInfo); // ListInfo
		  _menuState = 0;
		  printf("\nDownloading the music list from server...\n\n");
		}
		else
		{
		  printf("\n");
		  for (int i = 0; i < _songListSize; i++)
		  {
			printf("[%d] %s\n", i + 1, _songListNames[i]);
		  }
		}
	  }
	  else if (cmd[0] == 'q' || cmd[0] == 'Q')
	  {
		connectionStatus = Exit;
		break;
	  }
	  else
	  {
		int number = atoi(cmd);
		if (number > 0 && number <= _songListSize)
		{
		  _songListIndex = number - 1;
		  _setSendingBuffer(Song); // FileInfo
		  _menuState = 0;
		  printf("\nInitializing...\n\n");
		}
	  }
	}
	else if (_menuState == 2) // PlayMenu
	{
	  printf("\n[P] play\n");
	  printf("[S] stop\n");
	  printf("[A] pause\n");
	  printf("[X] main menu\n");
	  while (true)
	  {
		printf("Your choice: ");
		scanf("%s", &cmd);
		if (cmd[0] == 'p' || cmd[0] == 'P')
		{
		  if (!_audio.IsAvailable())
		  {
			_setSendingBuffer(Song); // FileInfo
			_menuState = 0;
			while (!_audio.IsAvailable()) Sleep(500);
		  }
		  _audio.Play();
		}
		else if (cmd[0] == 's' || cmd[0] == 'S')
		{
		  _audio.Stop();
		  _setSendingBuffer(Song); // FileInfo
		  _menuState = 0;
		  break;
		}
		else if (cmd[0] == 'a' || cmd[0] == 'A')
		{
		  _audio.Pause();
		}
		else if (cmd[0] == 'x' || cmd[0] == 'X')
		{
		  _audio.Stop();
		  _menuState = 1;
		  break;
		}
	  }
	}
  }
}


void ClientSession::_setSendingBuffer(requestType type, int serialnameber)
{
  _request.requestCommand = type;
  if (serialnameber != -1) _request.dataAmountSent = serialnameber;

  if (_request.requestCommand == SongList) _request.index = _songNumber;
  else if (_request.requestCommand == Song) _request.index = _songListIndex;
  else if (_request.requestCommand == SongIndex) _request.index = _songIndex;

  DWORD dwWaitResult = WaitForSingleObject(_mutex, INFINITE);  // no time-out interval         

  memcpy(_sendingBuffer, &_request, sizeof(DataHolder));

  ReleaseMutex(_mutex);
}

void ClientSession::sendingDataHandler()
{
  while (connectionStatus == isAlive)
  {
	if (_request.requestCommand == Status)
	{
	  Sleep(500); // status
	}

	Sleep(2);
	if (_sendStop) continue;
	if (_request.requestCommand == SongIndex && !_audio.wrapperIsExist()) continue;

	DWORD dwWaitResult = WaitForSingleObject(_mutex, INFINITE);  // no time-out interval         

	int len = send(_clientSocket, _sendingBuffer, sizeof(DataHolder), 0);
	if (len == SOCKET_ERROR) connectionStatus = isDeleted;

	ReleaseMutex(_mutex);
  }
}

void ClientSession::receivingDataHandler()
{
  while (connectionStatus == isAlive)
  {
	try
	{
	  _sendStop = false;
	  int len = recv(_clientSocket, _receiveBuffer, 1024, 0);
	  _sendStop = true;

	  if (len <= 0)
	  {
		printf("\nrecv failed with error: %d\n", WSAGetLastError());
		connectionStatus = isDeleted;
		break;
	  }

	  int offset = 0;
	  if (_receivedDataSize == 0)
	  {
		if (len < sizeof(DataHolder)) continue;

		DataHolder* response = (DataHolder*)_receiveBuffer;
		if (response->starBites != 0x12345678) continue;
		if (response->stopBits != 0x87654321) continue;

		_receivedDataSize = response->dataSize;

		memcpy(_receivedData, _receiveBuffer, sizeof(DataHolder));
		_receivedIndex = sizeof(DataHolder);
		offset = sizeof(DataHolder);
	  }

	  int remain = min(len - offset, _receivedDataSize - _receivedIndex);
	  if (remain > 0)
	  {
		memcpy(_receivedData + _receivedIndex, _receiveBuffer + offset, remain);
		_receivedIndex += remain;
		offset += remain;
	  }

	  if (_receivedIndex >= _receivedDataSize)
	  {
		_parseData();
		_receivedIndex = 0;
		_receivedDataSize = 0;
	  }
	}
	catch (...)
	{
	  printf("\nrecv failed with error: %d\n", WSAGetLastError());
	  connectionStatus = isDeleted;
	  break;
	}
  }
}

void ClientSession::_parseData()
{
  DataHolder* response = (DataHolder*)_receivedData;
  if (response->requestCommand == ListInfo) // ListInfo
  {
	if (_songListNames != NULL)
	{
	  for (int i = 0; i < _songListSize; i++)
	  {
		if (_songListNames[i] != NULL) delete[] _songListNames[i];
	  }
	}
	_songListSize = response->count;
	_songListNames = (char**)(new char* [_songListSize]);
	for (int i = 0; i < _songListSize; i++) _songListNames[i] = NULL;
	_songNumber = 0;

	_setSendingBuffer(SongList, response->dataAmountSent); // ListBlock
  }
  else if (response->requestCommand == SongList) // ListBlock
  {
	if (_songListNames == NULL)
	{
	  _setSendingBuffer(ListInfo, response->dataAmountSent); // ListInfo
	  return;
	}

	if (_songNumber != _request.index)
	{
	  _setSendingBuffer(SongList, response->dataAmountSent); // ListBlock
	  return;
	}

	int offset = sizeof(DataHolder);
	while (offset < _receivedDataSize && _songNumber < _songListSize)
	{
	  DataOnList* listInfo = (DataOnList*)(_receivedData + offset);
	  if (listInfo->listIndex != _songNumber) break;
	  offset += sizeof(DataOnList);

	  if (_receivedDataSize - offset < listInfo->listSize) break;

	  _songListNames[_songNumber] = new char[listInfo->listSize];
	  memcpy(_songListNames[_songNumber], _receivedData + offset, listInfo->listSize);
	  printf("[%d] %s\n", _songNumber + 1, _songListNames[_songNumber]);
	  _songNumber++;
	  offset += listInfo->listSize;
	}

	if (_songNumber < _songListSize)
	{
	  _setSendingBuffer(SongList, response->dataAmountSent); // ListBlock
	  return;
	}

	_menuState = 1;
	_setSendingBuffer(Status, response->dataAmountSent); // Satatus
  }
  else if (response->requestCommand == Song) // FileInfo
  {
	_songSize = response->count;
	if (_songSize == -1)
	{
	  printf("[%s couldn't be read\n", _songListNames[_songListIndex]);
	  _menuState = 1;
	  _setSendingBuffer(Status, response->dataAmountSent); // Status
	  return;
	}
	else if (_songSize < 1024) printf("\n%s filesize: %d bytes\n", _songListNames[_songListIndex], _songSize);
	else printf("\n%s filesize: %d KB\n", _songListNames[_songListIndex], _songSize / 1024);

	_audio.Start();
	_songIndex = 0;
	_menuState = 2;
	_setSendingBuffer(SongIndex, response->dataAmountSent); // FileBlock
  }
  else if (response->requestCommand == SongIndex) // FileBlock
  {
	if (_audio.IsAvailable()) _menuState = 2;

	_dataSize = _receivedDataSize - sizeof(DataHolder);
	if (_songIndex != response->index)
	{
	  _setSendingBuffer(SongIndex, response->dataAmountSent); // FileBlock
	  return;
	}

	_audio.wrapperAddQueue(_receivedData + sizeof(DataHolder), _dataSize);
	_songIndex += _dataSize;

	if (_songIndex < _songSize)
	{
	  _setSendingBuffer(SongIndex, response->dataAmountSent); // ListBlock
	  return;
	}

	_audio.Done();
	_setSendingBuffer(Status, response->dataAmountSent); // Status
  }
}

