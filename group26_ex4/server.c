#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h> // todo remove if removing printf
#include <stdlib.h>

#include "server.h"

void InitServer(char *argv[]);
void HandleServer();
void CreateSocketBindAndListen();
void SetSockAddrInAndBind();
void SetSocketToListen();
void ConnectToClientsAndRunGame();
void WINAPI TicTacToeGameThread(LPVOID lpParam);
void ParseNewUserRequest(char *ReceivedData, int ClientIndex);
void CloseSocketsAndThreads();

void InitServer(char *argv[]) {
	Server.ListeningSocket = INVALID_SOCKET;
	int ClientIndex = 0;
	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		Server.ClientsSockets[ClientIndex] = INVALID_SOCKET;
		Server.ClientsThreadHandle[ClientIndex] = NULL;
		Server.ClientIndex[ClientIndex] = ClientIndex;
		Server.Players[ClientIndex].PlayerType = None;
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
	if (Server.ListeningSocket == INVALID_SOCKET) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: CreateSocketBindAndListen failed to create socket. Error Number is %d\n", WSAGetLastError());
		OutputMessageToWindowAndLogFile(Server.LogFilePtr, ErrorMessage);
		exit(ERROR_CODE);
	}

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
		sprintf(ErrorMessage, "Custom message: SetSocketToListen failed to set Socket to listen. Error Number is %d\n", WSAGetLastError());
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
				sprintf(ErrorMessage, "Custom message: ConnectToClients failed to accept. Error Number is %d\n", WSAGetLastError());
				OutputMessageToWindowAndLogFile(Server.LogFilePtr, ErrorMessage);
				CloseSocketsAndThreads(); // todo add/check print
				exit(ERROR_CODE);
			}
			Server.ClientsThreadHandle[ClientIndex] = CreateThreadSimple((LPTHREAD_START_ROUTINE)TicTacToeGameThread,
																		 &Server.ClientIndex[ClientIndex],
																		 &Server.ClientsThreadID[ClientIndex],
																		 Server.LogFilePtr);
			if (Server.ClientsThreadHandle[ClientIndex] == NULL) {
				CloseSocketsAndThreads(); // todo check if add function to handle error
				exit(ERROR_CODE);
			}
		}
	}
}

void WINAPI TicTacToeGameThread(LPVOID lpParam) {
	if (NULL == lpParam) {
		OutputMessageToWindowAndLogFile(Server.LogFilePtr, "Custom message: Error in TicTacToeGame. Received null pointer.\n");
		CloseSocketsAndThreads(); // todo add/check print
		exit(ERROR_CODE);
	}
	int *ClientIndex = (int*)lpParam;
	char *ReceivedData = ReceiveData(Server.ClientsSockets[*ClientIndex], Server.LogFilePtr);
	if (ReceivedData == NULL) {
		CloseSocketsAndThreads();
		exit(ERROR_CODE);
	}
	ParseNewUserRequest(ReceivedData, *ClientIndex);
	Server.Players[*ClientIndex].PlayerType = *ClientIndex == 0 ? X : O;
	char NewUserAcceptedMessage[MESSAGE_LENGTH];
	if (Server.Players[*ClientIndex].PlayerType == X) {
		sprintf(NewUserAcceptedMessage, "NEW_USER_ACCEPTED:x;1");
	}
	else {
		sprintf(NewUserAcceptedMessage, "NEW_USER_ACCEPTED:o;2");
	}
	int SendDataToServerReturnValue;
	SendDataToServerReturnValue = SendData(Server.ClientsSockets[*ClientIndex], NewUserAcceptedMessage, Server.LogFilePtr);
	if (SendDataToServerReturnValue == ERROR_CODE) {
		CloseSocketsAndThreads();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent NEW_USER_ACCEPTED to Client %d, UserName %s.\n",
						  *ClientIndex, Server.Players[*ClientIndex].UserName);
	OutputMessageToWindowAndLogFile(Server.LogFilePtr, TempMessage);
}

void ParseNewUserRequest(char *ReceivedData, int ClientIndex) {
	int StartPosition = 0;
	int EndPosition = 0;
	int ParameterSize;
	while (ReceivedData[EndPosition] != ':') { // assuming valid input
		EndPosition++;
	}
	ParameterSize = (EndPosition - 1) - StartPosition + 1;
	if (strncmp(ReceivedData, "NEW_USER_REQUEST", ParameterSize) != 0) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: Got unexpected data from client number %d. Exiting...\n", ClientIndex);
		OutputMessageToWindowAndLogFile(Server.LogFilePtr, ErrorMessage);
		free(ReceivedData);
		CloseSocketsAndThreads();
		exit(ERROR_CODE);
	}
	EndPosition++;
	StartPosition = EndPosition;
	while (ReceivedData[EndPosition] != '\n') { // assuming valid input
		EndPosition++;
	}
	ParameterSize = (EndPosition - 1) - StartPosition + 1;
	strncpy(Server.Players[ClientIndex].UserName, ReceivedData + StartPosition, ParameterSize);
	Server.Players[ClientIndex].UserName[EndPosition] = '\0';
	free(ReceivedData);
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Received NEW_USER_REQUEST from Client %d, UserName %s.\n",
						  ClientIndex, Server.Players[ClientIndex].UserName);
	OutputMessageToWindowAndLogFile(Server.LogFilePtr, TempMessage);
}

void CloseSocketsAndThreads() {
	int ClientIndex = 0;
	DWORD ret_val;
	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		CloseOneSocket(Server.ClientsSockets[ClientIndex], Server.LogFilePtr);
		CloseOneThreadHandle(Server.ClientsThreadHandle[ClientIndex], Server.LogFilePtr);
	}
	CloseOneSocket(Server.ListeningSocket, Server.LogFilePtr);
	CloseWsaData(Server.LogFilePtr);
}