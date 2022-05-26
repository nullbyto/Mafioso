#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

SOCKET startup();

void handle_client(SOCKET& ListenSocket);

void disconnect(SOCKET sock);