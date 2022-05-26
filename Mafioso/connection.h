#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

SOCKET connect();

void disconnect(SOCKET sock);
std::string recieve_data(SOCKET& ConnectSocket);