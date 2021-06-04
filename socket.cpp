#include "stdafx.h"
#include <stdlib.h>  
#include <string.h>  
#include <iostream>
#include <windowsx.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "socket.h"
#include "Game.h"



char GameStatus[DEFAULT_BUFLEN];
int role;
static SOCKET ConnectSocket = INVALID_SOCKET;
BOOL connected = false;
SOCKET ClientHandlerSocket[10];
int Clients = 1;
char bufferBoard[DEFAULT_BUFLEN];

void SendInput(char input) {
	if (send(ConnectSocket, (const char *)&input, (int)strlen((const char *)&input), 0) == SOCKET_ERROR && connected == true) {
		strcat(&GameStatus[0], " - Send Error");
	}
	strcat(&GameStatus[0], " - [Send: ");
	strcat(&GameStatus[0], (const char *)&input);
	strcat(&GameStatus[0], "]");
}

void SendBoard(int Client) {
	if (connected == true) {
		if (GetGameOver(Client) == false) {

			bufferBoard[0] = GetPlayerPositionY(Client);
			bufferBoard[1] = GetPlayerPositionX(Client);

			int x = GetScore(Client);

			memcpy(&bufferBoard[2], &Board[0][0], 100 * 100 - 2);

			send(ClientHandlerSocket[Client], &bufferBoard[0], DEFAULT_BUFLEN, 0);
			//strcat(&GameStatus[0], " - sending Board");
		}
		else {
			memset(&bufferBoard, 0, sizeof(bufferBoard));
			bufferBoard[0] = '!';
			send(ClientHandlerSocket[Client], &bufferBoard[0], DEFAULT_BUFLEN, 0);
		}
	}
}

VOID CALLBACK ClientHandler(HWND lpParam, BOOLEAN TimerOrWaitFired) {
	int Client = Clients;
	int Result;
	int SendResult;
	connected = true;

	do {
		char recvbuf[DEFAULT_BUFLEN];
		ZeroMemory(&recvbuf, DEFAULT_BUFLEN);

		Result = recv(ClientHandlerSocket[Client], recvbuf, DEFAULT_BUFLEN, 0);
		if (Result > 0) {

			//strcat(&GameStatus[0], " - [Received= ");
			//strcat(&GameStatus[0], recvbuf);
			//strcat(&GameStatus[0], "]");

			SetInput(recvbuf[0], Client);
		}
		else if (Result == 0) {
			//printf("Connection closing...\n");
			return;
		}
		else {
			//printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientHandlerSocket[Client]);
			return;
		}
	} while (Result > 0);

	shutdown(ClientHandlerSocket[Client], SD_SEND);
	closesocket(ClientHandlerSocket[Client]);
}

VOID CALLBACK SocketServer(HWND lpParam, BOOLEAN TimerOrWaitFired) {

	role = SERVER;
	int Error = NOERROR;
	char sendbuf[DEFAULT_BUFLEN];
	int Result;
	int SendResult;
	ZeroMemory(sendbuf, DEFAULT_BUFLEN);
	HANDLE hTimerQueue = NULL;
	HANDLE hTimer = NULL;

	hTimerQueue = CreateTimerQueue();

	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	struct addrinfo *ServerConnectionInfo = NULL;
	struct addrinfo ServerProtocol;

	strcpy((char *)&GameStatus[0], (const char *) "Socket Server");


	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		Error = WSASTARTUPERROR;
		strcat((char *)&GameStatus[0], (const char *) " - WSA Startup Error");
		return;
	}

	ZeroMemory(&ServerProtocol, sizeof(ServerProtocol));
	ServerProtocol.ai_family = AF_INET;
	ServerProtocol.ai_socktype = SOCK_STREAM;
	ServerProtocol.ai_protocol = IPPROTO_TCP;
	ServerProtocol.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, DEFAULT_PORT, &ServerProtocol, &ServerConnectionInfo)) {
		Error = GETADDRINFOERROR;
		strcat((char *)&GameStatus[0], (const char *) " - Get Addr Info Error");
		return;
	}

	ListenSocket = socket(ServerConnectionInfo->ai_family, ServerConnectionInfo->ai_socktype, ServerConnectionInfo->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		Error = SOCKET_ERROR;
		strcat((char *)&GameStatus[0], (const char *)" - Socket Error");
		return;
	}

	if (bind(ListenSocket, ServerConnectionInfo->ai_addr, (int)ServerConnectionInfo->ai_addrlen) == SOCKET_ERROR) {
		Error = SOCKETBINDERROR;
		strcat((char *)&GameStatus[0], (const char *)" - Bind Error");
		return;
	}

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		Error = SOCKETLISTENERROR;
		strcat(&GameStatus[0], " - Listen Error");
		return;
	}

	while (1) {

		ClientHandlerSocket[Clients] = accept(ListenSocket, NULL, NULL);

		if (ClientHandlerSocket[Clients] != INVALID_SOCKET) {

			CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)ClientHandler, 0, 0, 0, WT_EXECUTEONLYONCE);
			Sleep(100);
			Clients++;

			strcat(&GameStatus[0], " - Accepted Connection");

		}
	}

	WSACleanup();

}


VOID CALLBACK SocketClient(HWND lpParam, BOOLEAN TimerOrWaitFired) {

	role = CLIENT;
	WSADATA wsaData;
	struct addrinfo ClientProtocol;
	struct addrinfo *ClientConnectionInfo = NULL;
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	ZeroMemory(sendbuf, DEFAULT_BUFLEN);
	ZeroMemory(recvbuf, DEFAULT_BUFLEN);
	int Result;
	int Error = NOERROR;

	char ip[] = "127.0.0.1";
	//char ip[] = "192.168.178.122";

	strcpy((char *)&GameStatus[0], (const char *) "Socket Client");

	// Initialize Winsock
	Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != 0) {
		Error = WSASTARTUPERROR;
		strcat(&GameStatus[0], " - WSA Startup Error");
		return;
	}

	ZeroMemory(&ClientProtocol, sizeof(ClientProtocol));
	ClientProtocol.ai_family = AF_INET;
	ClientProtocol.ai_socktype = SOCK_STREAM;
	ClientProtocol.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	if (getaddrinfo(ip, DEFAULT_PORT, &ClientProtocol, &ClientConnectionInfo)) {
		Error = GETADDRINFOERROR;
		strcat(&GameStatus[0], " - get addr info Error");
		return;
	}

	//// Attempt to connect to an address until one succeeds
	//for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ClientConnectionInfo->ai_family, ClientConnectionInfo->ai_socktype,
		ClientConnectionInfo->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		Error = SOCKET_ERROR;
		strcat(&GameStatus[0], " - Socket Error");
		return;
	}

	// Connect to server.
	Result = connect(ConnectSocket, ClientConnectionInfo->ai_addr, (int)ClientConnectionInfo->ai_addrlen);
	if (Result == SOCKET_ERROR) {
		Error = SOCKETCONNECTERROR;
		strcat(&GameStatus[0], " - Connect Error");
		return;
	}

	freeaddrinfo(ClientConnectionInfo);

	connected = true;

	//printf("Bytes Sent: %ld\n", Result);

	// Receive until the peer closes the connection
	do {
		Result = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if (Result > 0) {
			strcpy(&GameStatus[0], "Socket Client");

			if (recvbuf[0] != '!') {
				SetPlayerPostion((int)recvbuf[0], (int)recvbuf[1], IDPlayer);
				int i = 2;
				for (int y = 0; y < 100; y++) {
					for (int x = 0; x < 100; x++) {
						SetBoard(y, x, recvbuf[i]);
						i++;
					}
				}
			}
			else {
				strcpy(&GameStatus[0], "GameOver");
			}
		}
		else if (Result == 0) {
			//printf("Connection closed\n");
			return;
		}
		else {
			//printf("recv failed with error: %d\n", WSAGetLastError());
			return;
		}
	} while (Result > 0);

	// shutdown the connection since no more data will be sent
	Result = shutdown(ConnectSocket, SD_SEND);
	if (Result == SOCKET_ERROR) {
		Error = SOCKETSHUTDOWNERROR;
		strcat(&GameStatus[0], " - Shutdown Error");
		return;
	}

}