#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

SOCKET connect();

void disconnect(SOCKET sock);
int send_data(SOCKET ConnectSocket, const char* sendbuf);