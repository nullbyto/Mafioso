#include "connection.h"
//#include "room.h"
#include <iostream>

int main() {
	SOCKET ListenSocket;
	ListenSocket = startup();
	if (ListenSocket != NULL) {
		std::cout << "Server started successfully!" << std::endl;
	}

	//std::vector<Room> rooms;

	handle_client(ListenSocket);


	return 0;
}