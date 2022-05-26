#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

SOCKET connect();

void disconnect(SOCKET sock);
char* recieve_data(SOCKET& ConnectSocket);
bool send_data(SOCKET& ConnectSocket, const char* sendbuf);