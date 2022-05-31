#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include "room.h"

SOCKET startup();

void disconnect(SOCKET sock);

int recieve_data(SOCKET& ConnectSocket, std::vector<char> buf);

void handle_client(SOCKET& ListenSocket);