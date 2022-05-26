#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "5555"
#define SERVER_IP "127.0.0.1"
#define DEFAULT_BUFLEN 1024

void disconnect(SOCKET &socket) {
	// shutdown the send half of the connection since no more data will be sent
	int iResult = shutdown(socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
		return;
	}
	// cleanup
	closesocket(socket);
	WSACleanup();
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

void handle_client(SOCKET &ListenSocket) {
	SOCKET ClientSocket;
	int iResult;

	ClientSocket = INVALID_SOCKET;

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	char recv_buf[DEFAULT_BUFLEN];
	int recv_buf_len = DEFAULT_BUFLEN;
	int iSendResult;

	iResult = recv(ClientSocket, recv_buf, recv_buf_len, 0);
	if (iResult > 0) {
		//printf("Bytes received: %d\n", iResult);
		std::string msg;
		msg.append(recv_buf);
		std::cout << "Msg: " << msg.substr(0, iResult) << std::endl;

		// Echo the buffer back to the sender
		iSendResult = send(ClientSocket, recv_buf, iResult, 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}
		printf("Bytes sent: %d\n", iSendResult);
	}

	//// Receive until the peer shuts down the connection
	//do {

	//	iResult = recv(ClientSocket, recv_buf, recv_buf_len, 0);
	//	if (iResult > 0) {
	//		//printf("Bytes received: %d\n", iResult);
	//		std::string msg;
	//		msg.append(recv_buf);
	//		std::cout << "Msg: " << msg.substr(0, iResult) << std::endl;

	//		// Echo the buffer back to the sender
	//		iSendResult = send(ClientSocket, recv_buf, iResult, 0);
	//		if (iSendResult == SOCKET_ERROR) {
	//			printf("send failed: %d\n", WSAGetLastError());
	//			closesocket(ClientSocket);
	//			WSACleanup();
	//			return;
	//		}
	//		printf("Bytes sent: %d\n", iSendResult);
	//	}
	//	else if (iResult == 0)
	//		printf("Connection closing...\n");
	//	else {
	//		printf("recv failed: %d\n", WSAGetLastError());
	//		closesocket(ClientSocket);
	//		WSACleanup();
	//		return;
	//	}

	//} while (iResult > 0);


	disconnect(ClientSocket);
}

