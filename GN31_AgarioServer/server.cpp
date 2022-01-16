#include "server.h"

#include <random>

#include "packet.h"
#include "winsock-error.h"

GameServer GameServer::Instance;

GameServer::GameServer(){
}

void GameServer::init(){
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
        Player player{};
        // TODO 位置とか
        player.posX = rand() % 200;
        player.posY = rand() % 200;
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
    }
  }
}

void GameServer::uninit(){
  closesocket(socket);

  WSACleanup();
}

unsigned int GameServer::generatePlayerId(){
  std::mt19937 rand;
  while(true){
    unsigned int id = rand();
    if(players.find(id) == players.end()) return id;
  }
}
