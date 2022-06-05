#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

SOCKET connect();

void disconnect(SOCKET &sock);
int recieve_data(SOCKET &ConnectSocket, std::vector<char> &buf);
int send_data(SOCKET &ConnectSocket, const char* sendbuf);