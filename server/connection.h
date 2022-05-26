#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include "room.h"

SOCKET startup();

void handle_client(SOCKET& ListenSocket, std::vector<Room> rooms);

void disconnect(SOCKET sock);