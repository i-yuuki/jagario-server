#include "server.h"

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
        MessageBoxW(NULL, packet.name, L"C_JOIN received", MB_ICONINFORMATION);
        PacketServerJoin response{};
        response.playerId = rand();
        sendto(socket, reinterpret_cast<char*>(&response), sizeof(response), 0, reinterpret_cast<sockaddr*>(&senderAddr), senderAddrLen);
      }
        break;
      case PacketType::C_LEAVE:
        MessageBoxW(NULL, L"C_LEAVE received!", L"server", MB_ICONINFORMATION);
        break;
    }
  }
}

void GameServer::uninit(){
  closesocket(socket);

  WSACleanup();
}
