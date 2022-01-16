#pragma once

#include <unordered_map>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

#include "player.h"

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
  unsigned short port = 20250;
  char packetBuffer[65507];
  int maxPlayers = 100;
  std::unordered_map<unsigned int, Player> players;
  unsigned int generatePlayerId();
  template<class T>
  void broadcast(const T& packet){
    for(auto it = players.begin();it != players.end();it ++){
      sendto(socket, reinterpret_cast<const char*>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr*>(&it->second.address), sizeof(sockaddr));
    }
  }
};
