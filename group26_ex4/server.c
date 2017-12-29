#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h> // todo remove if removing printf
#include <stdlib.h>

#include "server.h"

void InitServer(char *argv[]);
void HandleServer();
void CreateSocketBindAndListen();
void SetSockAddrInAndBind();
void SetSocketToListen();
void ConnectToClientsAndRunGame();
void WINAPI TicTacToeGame(LPVOID lpParam);
void CloseSocketsAndThreads();

void InitServer(char *argv[]) {
	Server.ListeningSocket = INVALID_SOCKET;
	int ClientIndex = 0;
	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		Server.ClientsSockets[ClientIndex] = INVALID_SOCKET;
		Server.ClientsThreadHandle[ClientIndex] = NULL;
		Server.ClientIndex[ClientIndex] = ClientIndex;
	}
	Server.LogFilePtr = argv[1];
	Server.PortNum = atoi(argv[2]);
	InitLogFile(Server.LogFilePtr);
}

void HandleServer() {
	CreateSocketBindAndListen();
	ConnectToClientsAndRunGame(); // todo check name
}

void CreateSocketBindAndListen() {
	InitWsaData();
	Server.ListeningSocket = CreateOneSocket();
	SetSockAddrInAndBind();
	SetSocketToListen();
}

void SetSockAddrInAndBind() {
	int BindingReturnValue;

	Server.ListeningSocketService.sin_family = AF_INET;
	Server.ListeningSocketService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); // todo check
	Server.ListeningSocketService.sin_port = htons(Server.PortNum);
	BindingReturnValue = bind(Server.ListeningSocket, (SOCKADDR*)&Server.ListeningSocketService,
							  sizeof(Server.ListeningSocketService));
	if (BindingReturnValue != BINDING_SUCCEEDED) {
		CloseSocketsAndThreads(); // todo add print
		exit(ERROR_CODE);
	}
}

void SetSocketToListen() {
	int ListenReturnValue;
	ListenReturnValue = listen(Server.ListeningSocket, NUMBER_OF_CLIENTS); // todo check NUMBER_OF_CLIENTS
	if (ListenReturnValue != LISTEN_SUCCEEDED) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: SetSocketToListen failed to set Socket to listen.\n Error Number is %d\n", WSAGetLastError());
		OutputMessageToWindowAndLogFile(Server.LogFilePtr, ErrorMessage);
		CloseSocketsAndThreads(); // todo add/check print
		exit(ERROR_CODE);
	}
}

void ConnectToClientsAndRunGame() { // todo add support for more than one game / iteration
	int GameIndex = 0;
	int ClientIndex = 0;
	for (; GameIndex < NUMBER_OF_GAMES; GameIndex++) {
		for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) { // todo handle 1 minute wait / 5 minutes wait
			Server.ClientsSockets[ClientIndex] = accept(Server.ListeningSocket, NULL, NULL); // todo check NULL, NULL
			if (Server.ClientsSockets[ClientIndex] == INVALID_SOCKET) {
				char ErrorMessage[MESSAGE_LENGTH];
				sprintf(ErrorMessage, "Custom message: ConnectToClients failed to accept.\n Error Number is %d\n", WSAGetLastError());
				OutputMessageToWindowAndLogFile(Server.LogFilePtr, ErrorMessage);
				CloseSocketsAndThreads(); // todo add/check print
				exit(ERROR_CODE);
			}
			Server.ClientsThreadHandle[ClientIndex] = CreateThreadSimple((LPTHREAD_START_ROUTINE)TicTacToeGame,
																		 &Server.ClientIndex[ClientIndex],
																		 &Server.ClientsThreadID[ClientIndex],
																		 Server.LogFilePtr);
		}
	}
}

void WINAPI TicTacToeGame(LPVOID lpParam) {
	if (NULL == lpParam) {
		OutputMessageToWindowAndLogFile(Server.LogFilePtr, "Custom message: Error in TicTacToeGame. Received null pointer.\n");
	}
	int *ClientIndexPointer = (int*)lpParam;

}

void CloseSocketsAndThreads() {
	int ClientIndex = 0;
	DWORD ret_val;
	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		CloseOneSocket(Server.ClientsSockets[ClientIndex]);
		CloseOneThreadHandle(Server.ClientsThreadHandle[ClientIndex], Server.LogFilePtr);
	}
	CloseOneSocket(Server.ListeningSocket);
	CloseWsaData(Server.LogFilePtr);
}