#pragma once

#include "AudioPlayer.h"

enum requestType { Status, ListInfo, SongList, Song , SongIndex };
enum connectionType { isAlive = 1, isDeleted = 0, Exit = -1 };

struct DataHolder
{
  int starBites;		// 0x12345678 
  requestType requestCommand;
  int dataSize;
  int index;
  int count;
  int dataAmountSent;
  int stopBits;		//0x87654321 
};

struct DataOnList
{
  int listIndex;
  int listSize;
};

class ClientSession
{
public:
  ClientSession(SOCKET clientSocket);
  ~ClientSession(void);

  connectionType connectionStatus;
  void sendingDataHandler();
  void receivingDataHandler();
  void uiHandler();

private:
  HANDLE _mutex;
  SOCKET _clientSocket;
  char _receiveBuffer[1024];
  char _sendingBuffer[1024];
  char _receivedData[1024];
  int _receivedIndex;
  int _receivedDataSize;
  DataHolder _request;
  char** _songListNames;
  int _songListIndex;
  int _songListSize;
  int _songNumber;
  char _songData[1024];
  int _dataSize;
  int _songSize;
  int _songIndex;
  int _menuState; // 0:None, 1:MainMenu, 2:PlayMenu
  bool _sendStop;
  AudioPlayer _audio;

  void _setSendingBuffer(requestType type, int serialnameber = -1);
  void _parseData();
};

