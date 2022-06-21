#include <iostream>
#include <thread>
#include <future>
#include <list>

#include "connection.h"
#include "room.h"
#include "main.h"

int main() {
	SOCKET ListenSocket;
	ListenSocket = startup();
	if (ListenSocket != NULL) {
		std::cout << "Server started successfully!" << std::endl;
	}

	//////////////////////////////////////////

	std::list<SOCKET> clients;
	std::list<std::future<void>> clients_futures;

	SOCKET ClientSocket = INVALID_SOCKET;

	// Accept a client socket

	// THREADING
	/*while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			break;
		}
		pool.push_back(std::thread(handle_client, std::ref(ClientSocket), std::ref(clients)));
		clients_mutex.lock();
		clients.push_back(ClientSocket);
		clients_mutex.unlock();
		std::cout << clients.size() << "\n";
	}*/

	// ASYNC
	while (1) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			continue;
		}
		clients_futures.push_back(std::async(std::launch::async, handle_client, std::ref(ClientSocket), std::ref(clients)));
		std::cout << "Clients count: " << clients.size() << std::endl;
	}

	return 0;
}