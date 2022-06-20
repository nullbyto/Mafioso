#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <list>
#include "room.h"

SOCKET startup();

SOCKET startup_keepalive();

void disconnect(SOCKET sock);

int recieve_data(SOCKET ConnectSocket, std::vector<char> &buf);

void handle_client(SOCKET ListenSocket, std::list<SOCKET> &clients);