#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

#include "packet.h"

struct Player{
  float posX;
  float posY;
  float direction;
  float size;
  sockaddr_in address;
  char name[MAX_NAME_LENGTH];
};
