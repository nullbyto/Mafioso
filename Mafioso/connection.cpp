#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "5555"
#define SERVER_IP "127.0.0.1"
#define DEFAULT_BUFLEN 1024

bool send_data(SOCKET& ConnectSocket, const char* sendbuf) {
	int recvbuflen = DEFAULT_BUFLEN;

	//char recv_buf[DEFAULT_BUFLEN];

	int iResult;

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return false;
	}

	return true;

	//printf("Bytes Sent: %ld\n", iResult);

	//// shutdown the connection for sending since no more data will be sent
	//// the client can still use the ConnectSocket for receiving data
	//iResult = shutdown(ConnectSocket, SD_SEND);
	//if (iResult == SOCKET_ERROR) {
	//	printf("shutdown failed: %d\n", WSAGetLastError());
	//	closesocket(ConnectSocket);
	//	WSACleanup();
	//	return;
	//}

	//// Receive data until the server closes the connection
	//do {
	//	iResult = recv(ConnectSocket, recv_buf, recvbuflen, 0);
	//	if (iResult > 0)
	//		printf("Bytes received: %d\n", iResult);
	//	else if (iResult == 0)
	//		printf("Connection closed\n");
	//	else
	//		printf("recv failed: %d\n", WSAGetLastError());
	//} while (iResult > 0);
}

void shutdown_send(SOCKET &ConnectSocket) {
	/*shutdown the connection for sending since no more data will be sent
	the client can still use the ConnectSocket for receiving data*/
	int iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
}

std::string recieve_data(SOCKET &ConnectSocket) {
	char recv_buf[DEFAULT_BUFLEN];
	int recv_buf_len = DEFAULT_BUFLEN;
	int iResult;

	iResult = recv(ConnectSocket, recv_buf, recv_buf_len, 0);
	if (iResult > 0) {
		std::string msg;
		msg.append(recv_buf);
		return msg.substr(0, iResult);
	}
	else if (iResult == 0) {
		printf("Connection closed\n");
		return NULL;
	}
	else
		return NULL;

	printf("recv failed: %d\n", WSAGetLastError());

	//// Receive data until the server closes the connection
	//do {
	//	iResult = recv(ConnectSocket, recv_buf, recv_buf_len, 0);
	//	if (iResult > 0)
	//		printf("Bytes received: %d\n", iResult);
	//	else if (iResult == 0)
	//		printf("Connection closed\n");
	//	else
	//		printf("recv failed: %d\n", WSAGetLastError());
	//} while (iResult > 0);
}

SOCKET connect() {
	// Winsock initialization
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return NULL;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(SERVER_IP, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return NULL;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	ptr = result;
	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return NULL;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return NULL;
	}

	return ConnectSocket;

	const char* sendbuf = "this is a test";
	send_data(ConnectSocket, sendbuf);
}


void disconnect(SOCKET ConnectSocket) {
	// shutdown the send half of the connection since no more data will be sent
	int iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}
	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
}


//void connect() {
//	struct sockaddr_in server_addr;
//	char buffer[1024] = {0};
//
//	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	// Create TCP socket
//	if (sockfd < 0) {
//		std::cout << std::endl << "Socket creation error!" << std::endl;
//		return;
//	}
//
//	server_addr.sin_family	= AF_INET;
//	server_addr.sin_port = htons(PORT);
//
//	// Check if address is valid
//	if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
//		std::cout << std::endl << "Invalid address" << std::endl;
//		return;
//	}
//
//	SOCKET ConnectSocket = INVALID_SOCKET;
//
//	// Connect to server
//	auto connection = connect(sockfd, )
//	if () {
//
//	}
//}