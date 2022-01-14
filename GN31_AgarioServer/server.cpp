#include "server.h"

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

  sockaddr_in myaddr{};
  myaddr.sin_port = htons(port);
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(socket, (sockaddr*)&myaddr, sizeof(sockaddr)) == SOCKET_ERROR) throw std::runtime_error("Failed to bind socket (port in use?): " + getLastWinSockErrorMessage());
}

void GameServer::tick(){
}

void GameServer::uninit(){
  closesocket(socket);

  WSACleanup();
}
