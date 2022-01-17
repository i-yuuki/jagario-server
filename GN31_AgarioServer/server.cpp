#define NOMINMAX
#include "server.h"

#include <random>

#include "packet.h"
#include "winsock-error.h"

GameServer GameServer::Instance;

GameServer::GameServer(){
}

void GameServer::init(){
  populatePellets();

  WSADATA wd;
	WORD winSockVersion = MAKEWORD(2, 0);
  if(WSAStartup(winSockVersion, &wd)) throw std::runtime_error("WinSock failed to initialize: " + getLastWinSockErrorMessage());

  socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(socket == INVALID_SOCKET) throw std::runtime_error("Failed to create socket: " + getLastWinSockErrorMessage());

  unsigned long flag = 1;
	ioctlsocket(socket, FIONBIO, &flag);

  sockaddr_in myaddr{};
  myaddr.sin_port = htons(port);
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(socket, (sockaddr*)&myaddr, sizeof(sockaddr)) == SOCKET_ERROR) throw std::runtime_error("Failed to bind socket (port in use?): " + getLastWinSockErrorMessage());
}

void GameServer::tick(){
  sockaddr_in senderAddr;
  int senderAddrLen = sizeof(sockaddr);
  while(true){
    auto res = recvfrom(socket, packetBuffer, sizeof(packetBuffer), 0, reinterpret_cast<sockaddr*>(&senderAddr), &senderAddrLen);
    if(res == SOCKET_ERROR){
      int err = WSAGetLastError();
      if(err == WSAEWOULDBLOCK) break;
      // TODO エラーハンドル
      continue;
    }
    PacketType packetType;
    std::memcpy(&packetType, packetBuffer, sizeof(packetType));
    switch(packetType){
      case PacketType::C_JOIN:
      {
        PacketClientJoin& packet = *reinterpret_cast<PacketClientJoin*>(packetBuffer);

        unsigned int playerId = generatePlayerId();
        std::mt19937 rand(std::random_device{}());
        std::uniform_real_distribution<float> dist(initialPlayerSize / 2.0f, fieldSize - initialPlayerSize);
        Player player{};
        player.size = initialPlayerSize;
        setRandomSafePosition(player);
        player.address = senderAddr;
        strcpy_s(player.name, sizeof(player.name), packet.name);
        players.insert({playerId, player});

        for(auto it = players.begin();it != players.end();it ++){
          PacketServerAddPlayer packetAddPlayer;
          packetAddPlayer.playerId = it->first;
          packetAddPlayer.posX = it->second.posX;
          packetAddPlayer.posY = it->second.posY;
          packetAddPlayer.size = it->second.size;
          strcpy_s(packetAddPlayer.name, sizeof(packetAddPlayer.name), it->second.name);
          
          if(it->first == playerId){
            // 参加者を全プレイヤーに伝える
            broadcast(packetAddPlayer);
          }else{
            // 全プレイヤーを参加者に伝える
            sendto(socket, reinterpret_cast<char*>(&packetAddPlayer), sizeof(packetAddPlayer), 0, reinterpret_cast<sockaddr*>(&senderAddr), senderAddrLen);
          }
        }

        for(auto it = pellets.begin();it != pellets.end();it ++){
          PacketServerAddPellet packet{};
          packet.pelletId = it->first;
          packet.posX = it->second.posX;
          packet.posY = it->second.posY;
          sendto(socket, reinterpret_cast<char*>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr*>(&senderAddr), senderAddrLen);
        }

        PacketServerJoin response{};
        response.playerId = playerId;
        sendto(socket, reinterpret_cast<char*>(&response), sizeof(response), 0, reinterpret_cast<sockaddr*>(&senderAddr), senderAddrLen);
      }
        break;
      case PacketType::C_LEAVE:
      {
        PacketClientLeave& packet = *reinterpret_cast<PacketClientLeave*>(packetBuffer);
        players.erase(packet.playerId);

        PacketServerRemovePlayer packetRemovePlayer;
        packetRemovePlayer.playerId = packet.playerId;
        broadcast(packetRemovePlayer);
      }
        break;
      case PacketType::C_UPDATE:
      {
        PacketClientUpdate& packet = *reinterpret_cast<PacketClientUpdate*>(packetBuffer);
        if(auto* player = getPlayer(packet.playerId)){
          player->direction = packet.direction;
          player->boost = packet.boost;
        }
      }
        break;
    }
  }

  for(auto it = players.begin();it != players.end();){
    auto& player = it->second;
    float playerSizeHalf = player.size / 2.0f;
    float speed = 10;
    speed = std::max(3.0f, speed - (player.size - initialPlayerSize) / 30.0f);
    if(player.boost && player.size > initialPlayerSize){
      player.size = std::max(initialPlayerSize, player.size - 2);
      speed *= 4;
    }
    player.posX += std::sin(player.direction) * speed;
    player.posY += std::cos(player.direction) * speed;

    player.posX = std::min(std::max(player.posX, playerSizeHalf), fieldSize - playerSizeHalf);
    player.posY = std::min(std::max(player.posY, playerSizeHalf), fieldSize - playerSizeHalf);

    for(auto it1 = pellets.begin();it1 != pellets.end();){
      unsigned int pelletId = it1->first;
      auto& pellet = it1->second;
      float distX = player.posX - pellet.posX;
      float distY = player.posY - pellet.posY;
      float distSq = distX * distX + distY * distY;
      if(distSq < (playerSizeHalf * playerSizeHalf + pelletSize * pelletSize)){
        it1 = pellets.erase(it1);
        player.size ++;
        PacketServerRemovePellet packet{};
        packet.pelletId = pelletId;
        broadcast(packet);
      }else{
        it1 ++;
      }
    }

    bool die = false;
    for(auto it1 = players.begin();it1 != players.end();it1 ++){
      auto& p1 = it1->second;
      float p1SizeHalf = p1.size / 2.0f;
      float distX = player.posX - p1.posX;
      float distY = player.posY - p1.posY;
      float distSq = distX * distX + distY * distY;
      if(distSq < (playerSizeHalf * playerSizeHalf + p1SizeHalf * p1SizeHalf)){
        die = p1.size >= player.size + 15;
        if(die){
          p1.size += player.size / 2;
          break;
        }
      }
    }

    if(die){
      PacketServerRemovePlayer packet;
      packet.playerId = it->first;
      broadcast(packet);
      it = players.erase(it);
      continue;
    }

    PacketServerUpdatePlayer packet;
    packet.playerId = it->first;
    packet.posX = player.posX;
    packet.posY = player.posY;
    packet.size = player.size;
    broadcast(packet);

    it ++;
  }

  populatePellets();
}

void GameServer::uninit(){
  closesocket(socket);

  WSACleanup();
}

unsigned int GameServer::generatePlayerId(){
  std::mt19937 rand(std::random_device{}());
  while(true){
    unsigned int id = rand();
    if(players.find(id) == players.end()) return id;
  }
}

const std::unordered_map<unsigned int, Player>& GameServer::getPlayers(){
  return players;
}

Player* GameServer::getPlayer(unsigned int playerId){
  auto it = players.find(playerId);
  return it == players.end() ? nullptr : &it->second;
}

void GameServer::setRandomSafePosition(Player& player){
  std::mt19937 rand(std::random_device{}());
  std::uniform_real_distribution<float> dist(initialPlayerSize / 2.0f, fieldSize - initialPlayerSize);
  float playerSizeHalf = player.size / 2.0f;
  for(int i = 0;i < 100;i ++){
    player.posX = dist(rand);
    player.posY = dist(rand);
    for(auto it = players.begin();it != players.end();it ++){
      auto& p1 = it->second;
      float p1SizeHalf = p1.size / 2.0f;
      float distX = player.posX - p1.posX;
      float distY = player.posY - p1.posY;
      float distSq = distX * distX + distY * distY;
      if(distSq < (playerSizeHalf * playerSizeHalf + p1SizeHalf * p1SizeHalf + 100 * 100)){
        continue;
      }
      return;
    }
  }
}

void GameServer::addPellet(){
  unsigned int id;
  std::mt19937 rand(std::random_device{}());
  while(true){
    id = rand();
    if(pellets.find(id) == pellets.end()) break;
  }

  std::uniform_real_distribution<float> dist(0, fieldSize);
  Pellet pellet{};
  pellet.posX = dist(rand);
  pellet.posY = dist(rand);
  pellets.insert({id, pellet});

  PacketServerAddPellet packet{};
  packet.pelletId = id;
  packet.posX = pellet.posX;
  packet.posY = pellet.posY;
  broadcast(packet);
}

void GameServer::populatePellets(){
  while(pellets.size() < pelletCount){
    addPellet();
  }
}
