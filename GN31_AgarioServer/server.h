#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

class GameServer{
public:
  static GameServer Instance;
  void init();
  void tick();
  void uninit();
private:
  GameServer();
private:
  SOCKET socket = NULL;
  int maxPlayers = 100;
};
