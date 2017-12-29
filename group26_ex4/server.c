#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h> // todo remove if removing printf
#include <stdlib.h>

#include "server.h"

void InitServer(char *argv[]);
void HandleServer();
void CreateSocketBindAndListen();
void InitWsaData();
void SetSockAddrInAndBind();
void SetSocketToListen();
void ConnectToClients();
HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id);
void WINAPI TicTacToeGame(LPVOID lpParam);
void InitLogFile();
void WriteToLogFile(char *MessageToWrite);
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
	InitLogFile();
}

void HandleServer() {
	CreateSocketBindAndListen();
	ConnectToClients(); // todo check name
}

void CreateSocketBindAndListen() {
	InitWsaData();
	Server.ListeningSocket = CreateOneSocket();
	SetSockAddrInAndBind();
	SetSocketToListen();
}

void InitWsaData() {
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)	{
		printf("Error %ld at WSAStartup().\nExiting...\n", StartupRes);
		// Tell the user that we could not find a usable WinSock DLL.
		exit(ERROR_CODE); // todo handle error
	}
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
		printf("SetSocketToListen failed to set Socket to listen.\n Error Number is %d\n", WSAGetLastError());
		CloseSocketsAndThreads(); // todo add/check print
		exit(ERROR_CODE);
	}
}

void ConnectToClients() { // todo add support for more than one game / iteration
	int GameIndex = 0;
	int ClientIndex = 0;
	for (; GameIndex < NUMBER_OF_GAMES; GameIndex++) {
		for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) { // todo handle 1 minute wait / 5 minutes wait
			Server.ClientsSockets[ClientIndex] = accept(Server.ListeningSocket, NULL, NULL); // todo check NULL, NULL
			if (Server.ClientsSockets[ClientIndex] == INVALID_SOCKET) {
				printf("ConnectToClients failed to accept.\n Error Number is %d\n", WSAGetLastError());
				CloseSocketsAndThreads(); // todo add/check print
				exit(ERROR_CODE);
			}
			Server.ClientsThreadHandle[ClientIndex] = CreateThreadSimple((LPTHREAD_START_ROUTINE)TicTacToeGame,
																		 &Server.ClientIndex[ClientIndex],
																		 &Server.ClientsThreadID[ClientIndex]);
		}
	}
}

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id) {
	HANDLE thread_handle;

	if (NULL == p_start_routine) {
		WriteToLogFile("Error when creating a thread. Received null pointer.\n");
		CloseSocketsAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}

	if (NULL == p_thread_id) {
		WriteToLogFile("Error when creating a thread. Received null pointer.\n");
		CloseSocketsAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}

	thread_handle = CreateThread(
		NULL,                /*  default security attributes */
		0,                   /*  use default stack size */
		p_start_routine,     /*  thread function */
		p_thread_parameters, /*  argument to thread function */
		0,                   /*  use default creation flags */
		p_thread_id);        /*  returns the thread identifier */

	if (NULL == thread_handle) {
		WriteToLogFile("Couldn't create thread.\n");
		CloseSocketsAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}

	return thread_handle;
}

void WINAPI TicTacToeGame(LPVOID lpParam) {
	if (NULL == lpParam) {
		WriteToLogFile("Error in TicTacToeGame. Received null pointer.\n");
	}
	int *ClientIndexPointer = (int*)lpParam;

}

void InitLogFile() {
	FILE *LogFilePointer = fopen(Server.LogFilePtr, "w");
	if (LogFilePointer == NULL) {
		printf("Couldn't open log file.\n");
		exit(ERROR_CODE);
	}
	fclose(LogFilePointer);
}

void WriteToLogFile(char *MessageToWrite) { // todo check all calls to this function
	FILE *LogFilePointer = fopen(Server.LogFilePtr, "a");
	if (LogFilePointer == NULL) {
		printf("Couldn't open log file.\n");
		CloseSocketsAndThreads();
		exit(ERROR_CODE);
	}
	fputs(MessageToWrite, LogFilePointer);
	fclose(LogFilePointer);
}

void CloseSocketsAndThreads() {
	int ClientIndex = 0;
	DWORD ret_val;
	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		CloseOneSocket(Server.ClientsSockets[ClientIndex]);
		ret_val = CloseHandle(Server.ClientsThreadHandle[ClientIndex]);
		if (FALSE == ret_val) {
			WriteToLogFile("Error when closing threads.\n");
		}
	}
	CloseOneSocket(Server.ListeningSocket);
}