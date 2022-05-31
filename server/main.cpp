#include "connection.h"
#include "room.h"
#include <iostream>
#include <thread>
#include <future>

int main() {
	SOCKET ListenSocket;
	ListenSocket = startup();
	if (ListenSocket != NULL) {
		std::cout << "Server started successfully!" << std::endl;
	}

	/*std::vector<Room> rooms;*/

	//////////////////////////////////////////

	std::vector<std::thread> pool;

	SOCKET ClientSocket;
	int iResult;

	ClientSocket = INVALID_SOCKET;

	// Accept a client socket
	/*int clients = 0;*/
	// THREADING
	while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}
		pool.push_back(std::thread(handle_client, std::ref(ClientSocket)));
	}
	// ASYNC
	/*while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}
		auto future = std::async(handle_client, std::ref(ClientSocket));
	}*/


	return 0;
}