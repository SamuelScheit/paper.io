#pragma once

#define NOERROR 0
#define WSASTARTUPERROR 1
#define SOCKETLISTENERROR 2
#define SCOKETBINDERROR 3
#define SOCKETSHUTDOWNERROR 4
#define SOCKETCONNECTERROR 5
#define SOCKETBINDERROR 6
#define GETADDRINFOERROR 7

#define SERVER 1
#define CLIENT 0

#define DEFAULT_BUFLEN 10512
#define DEFAULT_PORT "100"
extern char GameStatus[DEFAULT_BUFLEN];
extern int role;
extern int Clients;

#pragma comment (lib, "Ws2_32.lib")

VOID CALLBACK SocketClient(HWND lpParam, BOOLEAN TimerOrWaitFired);
VOID CALLBACK SocketServer(HWND lpParam, BOOLEAN TimerOrWaitFired);
void SendInput(char input);
void SendBoard(int Client);
