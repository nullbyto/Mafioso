#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <thread>
#include <mutex>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "room.h"
#include "main.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT "5555"
#define SERVER_IP "127.0.0.1"
#define DEFAULT_BUFLEN 1024

static int done = 0;
static bool first = true;
static std::mutex first_mutex;
static SOCKET leader = NULL;

static Room room = {};
static nlohmann::json roomJSON;
static std::mutex room_mutex;

void disconnect(SOCKET &socket, std::list<SOCKET> &clients) {
	// shutdown the send half of the connection since no more data will be sent
	int iResult = shutdown(socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
		clients.pop_back();
		return;
	}
	// cleanup
	closesocket(socket);
	WSACleanup();
	clients.pop_back();
}

SOCKET startup() {
	// Winsock initialization
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return NULL;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return NULL;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return NULL;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return NULL;
	}
	return ListenSocket;
}

SOCKET startup_keepalive() {
	// Winsock initialization
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return NULL;
	}

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return NULL;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return NULL;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	// Set keepalive option
	int optval;
	socklen_t optlen = sizeof(optval);
	if (getsockopt(ListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char *) & optval, &optlen) < 0) {
		closesocket(ListenSocket);
		return NULL;
	}

	// Set the option active
	optval = 1;
	optlen = sizeof(optval);
	if (setsockopt(ListenSocket, SOL_SOCKET, SO_KEEPALIVE, (char *) & optval, optlen) < 0) {
		perror("setsockopt()");
		closesocket(ListenSocket);
		return NULL;
	}

	optval = 2;
	if (setsockopt(ListenSocket, IPPROTO_TCP, TCP_KEEPCNT, (char *)&optval, sizeof(optval))) { perror("ERROR: setsocketopt(), SO_KEEPCNT"); exit(0); };
	optval = 2;
	if (setsockopt(ListenSocket, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&optval, sizeof(optval))) { perror("ERROR: setsocketopt(), SO_KEEPINTVL"); exit(0); };

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return NULL;
	}
	return ListenSocket;
}

char* getipaddr(SOCKET s, bool port)
{
	sockaddr_in SockAddr;
	int addrlen = sizeof(SockAddr);

	if (getsockname(s, (LPSOCKADDR)&SockAddr, &addrlen) == SOCKET_ERROR)
	{
		//err = WSAGetLastError();
		return NULL;
	}

	if (port)
	{
		char* portstr = new char[6];
		sprintf_s(portstr, 6, "%d", htons(SockAddr.sin_port));

		return portstr;
	}

	char* ipstr = new char[16];
	inet_ntop(AF_INET, &SockAddr.sin_addr, ipstr, 16);

	return ipstr;
}

int recieve_data(SOCKET &ConnectSocket, std::vector<char> &buf) {
	return recv(ConnectSocket, &buf[0], buf.size(), 0);
}

int send_data(SOCKET& ConnectSocket, const char* sendbuf) {
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;

	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	return iResult;
}

void broadcast_data(std::list<SOCKET> &clients, const char* sendbuf, SOCKET exception) {
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;

	for (auto client : clients) {
		if (client == exception)
			continue;
		iResult = send(client, sendbuf, strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(client);
			WSACleanup();
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.remove(client);
			return;
		}
	}
	return;
}

int recieve_setup(SOCKET& ClientSocket, std::list<SOCKET>& clients) {
	std::vector<char> room_buf(1024);
	std::string room_json_str;
	auto ip_str = getipaddr(ClientSocket, 0);
	int iResult = 0;
	if (iResult <= 0) {
		iResult = recieve_data(ClientSocket, room_buf);
		if (iResult == 0 || iResult == -1) {
			std::cout << "[" << ip_str << "] disconnected" << std::endl;
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.remove(ClientSocket);
			return iResult;
		}
	}
	room_json_str.append(room_buf.cbegin(), room_buf.cend());

	json room_json = json::parse(room_json_str);

	Roles roles = {
		room_json["roles"]["villager"],
		room_json["roles"]["doctor"],
		room_json["roles"]["cop"],
		room_json["roles"]["escort"],
		room_json["roles"]["armsdealer"],
		room_json["roles"]["godfather"],
		room_json["roles"]["mafioso"],
		room_json["roles"]["jester"],
	};

	Settings settings = {
		room_json["settings"]["day_length"],
		room_json["settings"]["night_length"],
		room_json["settings"]["last_will"],
		room_json["settings"]["no_reveal"],
		room_json["settings"]["day_start"],
	};

	room_mutex.lock();
	room.roles = roles;
	room.settings = settings;
	roomJSON = room_json;
	room_mutex.unlock();

	std::cout << room_json_str << std::endl;

	return iResult;
}

void handle_client(SOCKET ClientSocket, std::list<SOCKET> &clients) {
	int iResult = 0;

	/////////////////////////////////////////////////////////////////
	// Recieve name
	
	std::vector<char> name_buf(512);
	std::string client_name;

	auto ip_str = getipaddr(ClientSocket, 0);
	if (iResult <= 0) {
		iResult = recieve_data(ClientSocket, name_buf);
		if (iResult == 0 || iResult == -1) {
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}
	}
	client_name.append(name_buf.cbegin(), name_buf.cend());
	std::cout << "[" << ip_str << "] " << client_name << " joined the server" << std::endl;

	/////////////////////////////////////////////////////////////////
	// Send room info

	clients_mutex.lock();
	clients.push_back(ClientSocket);
	int clients_count = clients.size();
	clients_mutex.unlock();

	iResult = 0;
	if (iResult <= 0) {
		iResult = send(ClientSocket, (char*)&clients_count, sizeof(clients_count), 0);
		if (iResult == 0) {
			std::cout << "[" << ip_str << "] disconnected" << std::endl;
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.remove(ClientSocket);
			return;
		}
	}

	/////////////////////////////////////////////////////////////////
	// Recieve room setup info
	// and broadcast room setup if room already set

	if (clients_count == 1) {
		leader = ClientSocket;
		if (recieve_setup(ClientSocket, clients) > 0) {
			broadcast_data(clients, roomJSON.dump().data(), ClientSocket);
			done = 1;
			return;
		}
	}
	else {
		// Wait until setup is done
		while (done == 0) {

		}
		broadcast_data(clients, roomJSON.dump().data(), leader);
	}

	while (1) {

	}

	return;
}
