#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>
#include <iostream>


#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")


class Client
{

public:
	
	// constructor/de-constructor
	Client(SOCKET clientSocket);
	~Client(void);

	// methods

	void Receive();
	int receive_till_zero(SOCKET sock, wchar_t* tmpbuf, int& numbytes);
	void UserRequests();

	// active client
	int _isActive;

private:

	unsigned long timeout;
	SOCKET _activeSocket;
	IDirectSound8* directSound = nullptr;
	IDirectSoundBuffer* primaryBuffer = nullptr;
	IDirectSoundBuffer8* secondaryBuffer = nullptr;
	BYTE* dataBuffer = nullptr;
	DWORD dataBufferSize;
	DWORD averageBytesPerSecond;

	static void StaticReceive(LPVOID param);
	static void StaticUserRequests(LPVOID param);

	HANDLE threadMutex;

	std::vector<std::wstring> _storedList;
	int _totalCount;
	int _commandType;
	std::string _checkTrack;
	std::wstring _selectedTrack;
	std::string _selectTrackNum;
	std::string _userinput;
	bool CheckForNumberInput(std::string input);
	void deserialize(std::vector<std::string> &restore, char* buffer, int total_count);
	bool ReceiveData(char * data, int totalbytes);
	void SendRequests(int commandType);
	void DefineMenu(int choice);
	void ViewStoredList();
	bool PlayWaveFile(BYTE* recvbuffer);
	bool InitializePlayer(BYTE* recvbuffer);
	std::vector<std::wstring> Split(std::wstring stringToSplit, wchar_t delimeter);
	void getSongData();
	std::string selectSong(std::string input);
};
