#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include <future>
#include <stdlib.h>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "room.h"
#include "main.h"
#include "connection.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT "5555"
#define SERVER_IP "127.0.0.1"
#define DEFAULT_BUFLEN 1024

#define FLAG_FAIL "!#"
#define FLAG_CHAT "0#"
#define FLAG_GAME "1#"
#define FLAG_INFO "2#"
#define FLAG_ROOM "3#"

static bool done = false;
static std::mutex done_mutex;
static SOCKET leader = NULL;

static Room room = {};
static nlohmann::json roomJSON;
static std::mutex room_mutex;

static std::mutex players_mutex;
static std::list<Player> players;//////////
static json players_json;

void disconnect(SOCKET socket, std::list<SOCKET> &clients) {
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

int recieve_data(SOCKET ConnectSocket, std::vector<char> &buf) {
	return recv(ConnectSocket, &buf[0], (int)buf.size(), 0);
}

int send_data(SOCKET ConnectSocket, const char* sendbuf) {
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;

	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	return iResult;
}

void broadcast_data(std::list<SOCKET> &clients, const char* sendbuf, SOCKET exception) {
	int iResult = 0;

	for (auto client : clients) {
		if (client == exception)
			continue;
		iResult = send(client, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(client);
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.remove(client);
			return;
		}
	}
	return;
}

int recieve_setup(SOCKET ClientSocket, std::list<SOCKET>& clients) {
	std::vector<char> room_buf(1024);
	std::string room_json_str;
	auto ip_str = getipaddr(ClientSocket, 0);
	int iResult = 0;
	iResult = recieve_data(ClientSocket, room_buf);
	if (iResult == 0 || iResult == -1) {
		std::cout << "[" << ip_str << "] disconnected" << std::endl;
		closesocket(ClientSocket);
		clients_mutex.lock();
		clients.remove(ClientSocket);
		clients_mutex.unlock();
		return iResult;
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

	{
		std::lock_guard lock(room_mutex);
		room.roles = roles;
		room.settings = settings;
		roomJSON = room_json;
	}

	std::cout << room_json_str << std::endl;

	return iResult;
}

bool is_player_name_taken(std::string name) {
	for (auto player : players) {
		if (player.name == name) {
			return true;
		}
	}
	return false;
}

void handle_chat(SOCKET ClientSocket, std::string msg, std::list<SOCKET> &clients) {
	std::string msg_raw = msg.substr(2, msg.size());
	//std::cout << raw_msg << std::endl;
	broadcast_data(clients, msg.data(), NULL);
}

void handle_client(SOCKET ClientSocket, std::list<SOCKET> &clients) {
	int iResult = 0;

	/////////////////////////////////////////////////////////////////
	// Recieve name

	std::string client_name;
	int done_name = 0;
	auto ip_str = getipaddr(ClientSocket, 0);

	// Check if player name is taken and send information
	while (!done_name) {
		std::vector<char> name_buf(512);
		client_name = "";

		iResult = 0;
		iResult = recieve_data(ClientSocket, name_buf);
		if (iResult == 0 || iResult == -1) {
			closesocket(ClientSocket);
			return;
		}
		client_name.append(name_buf.cbegin(), name_buf.cend());
		client_name.erase(std::find(client_name.begin(), client_name.end(), '\0'), client_name.end());

		if (!is_player_name_taken(client_name)) {
			done_name = 1;
		}
		iResult = send(ClientSocket, (char*)&done_name, sizeof(done_name), 0);
		if (iResult == 0) {
			std::cout << "[" << ip_str << "] disconnected" << std::endl;
			{
				std::lock_guard lock(clients_mutex);
				clients.remove(ClientSocket);
			}
			return;
		}
	}
	std::cout << "[" << ip_str << "] " << client_name << " joined the server" << std::endl;

	// Add player struct to json
	Player p = Player(client_name, R_NONE);
	json p_json = {
		{"name", p.name}, {"id", p.id}, {"role", p.role}
	};
	{
		std::lock_guard lock(players_mutex);
		players.push_back(p);
	}

	/////////////////////////////////////////////////////////////////
	// Send room info

	{
		std::lock_guard lock(clients_mutex);
		clients.push_back(ClientSocket);
	}
	int clients_count = (int)clients.size();

	iResult = 0;
	iResult = send(ClientSocket, (char*)&clients_count, sizeof(clients_count), 0);
	if (iResult == 0) {
		std::cout << "[" << ip_str << "] disconnected" << std::endl;
		{
			std::lock_guard lock(clients_mutex);
			clients.remove(ClientSocket);
		}
		return;
	}

	
	/////////////////////////////////////////////////////////////////
	// Recieve room setup info
	// and broadcast room setup if room already set

	{ // lock other clients handlers waiting to recieve the setup
		std::lock_guard lock(done_mutex);
		if (!done) {
			leader = ClientSocket;
			if (recieve_setup(ClientSocket, clients) > 0) {
				// Set flag to be done so other clients know a room has been created
				done = true;
			}
			// if recieving setup failed cuz of disconnect then choose another client
			else {
				if (clients.size() > 0) {
					auto first_client = clients.begin();
					std::string failed = FLAG_FAIL;
					send_data(*first_client, failed.data());
				}
				return;
			}
		}
	}

	{
		std::lock_guard lock(room_mutex);
		room.players = players;
		roomJSON["players"] += p_json;
	}
	// Broadcast updated roomJSON with players
	std::string room_json_str = FLAG_INFO;
	room_json_str += roomJSON.dump();
	broadcast_data(clients, room_json_str.data(), NULL);

	// This is to temp "fix" recieving/sending data streams to not get mixed up together
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(0.001s);

	// Broadcast joined msg
	std::string joined_msg = FLAG_CHAT;
	joined_msg += "[Server]: " + client_name + " joined the server\n";
	broadcast_data(clients, joined_msg.data(), NULL);

	/////////////////////////////////////////////////////////////////
	// Loop gamestate + chat

	while (1) {
		std::vector<char> data_buf(DEFAULT_BUFLEN);
		std::string data;

		iResult = 0;
		iResult = recv(ClientSocket, data_buf.data(), data_buf.size(), 0);
		if (iResult == 0 || iResult == -1) {
			std::cout << "[" << ip_str << "] disconnected" << std::endl;
			{
				std::lock_guard lock(clients_mutex);
				clients.remove(ClientSocket);
			}
			closesocket(ClientSocket);
			
			{
				std::lock_guard lock(players_mutex);
				players.remove(p);
			}

			std::string left_msg = FLAG_CHAT;
			left_msg += "[Server]: " + client_name + " left the server\n";

			broadcast_data(clients, left_msg.data(), NULL);

			// ########################################!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			// CONTINUE HERE
			// broadcast player left to update player list.
			// ########################################!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			//// This is to temp "fix" recieving/sending data streams to not get mixed up together
			//using namespace std::chrono_literals;
			//std::this_thread::sleep_for(0.001s);

			//// Broadcast updated roomJSON with updated players
			//std::string room_json_str = FLAG_INFO;
			//room_json_str += roomJSON.dump();
			//broadcast_data(clients, room_json_str.data(), NULL);
			return;
		}
		data.append(data_buf.cbegin(), data_buf.cend());
		std::string data_raw = data.substr(2, data.size());

		auto prefix = data.substr(0, 2);

		if (prefix == FLAG_CHAT) {
			handle_chat(ClientSocket, data, clients);
		}
		else if (prefix == FLAG_GAME) {
			std::cout << "ma3\n";
		}
		else if (prefix == FLAG_INFO) {
			broadcast_data(clients, data.data(), NULL);
		}
	}


	return;
}
