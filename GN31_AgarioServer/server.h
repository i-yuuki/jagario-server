#pragma once

#include <unordered_map>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

#include "player.h"
#include "pellet.h"

class GameServer{
public:
  static GameServer Instance;
  void init();
  void tick();
  void uninit();
  const std::unordered_map<unsigned int, Player>& getPlayers();
private:
  GameServer();
  SOCKET socket = NULL;
  unsigned short port = 20250;
  char packetBuffer[65507];
  int maxPlayers = 100;
  int initialPlayerSize = 10;
  float fieldSize = 2000;
  int pelletCount = 300;
  float pelletSize = 10;
  std::unordered_map<unsigned int, Player> players;
  std::unordered_map<unsigned int, Pellet> pellets;
  unsigned int generatePlayerId();
  Player* getPlayer(unsigned int playerId);
  // できるだけ安全なランダム位置に移動
  void setRandomSafePosition(Player& player);
  void addPellet();
  void populatePellets();
  template<class T>
  void broadcast(const T& packet){
    for(auto it = players.begin();it != players.end();it ++){
      sendto(socket, reinterpret_cast<const char*>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr*>(&it->second.address), sizeof(sockaddr));
    }
  }
};
