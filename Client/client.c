#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>

#include "client.h"

void InitClient(char *argv[]);
void HandleClient();
void ConnectToServer();
void CloseSocketAndThreads();


void InitClient(char *argv[]) {
	Client.Socket = INVALID_SOCKET;
	Client.LogFilePtr = argv[1];
	Client.ServerIP = argv[2];
	Client.ServerPortNum = atoi(argv[3]);
	Client.UserName = argv[4];
	Client.SendThreadHandle = NULL;
	Client.ReceiveThreadHandle = NULL;
	Client.UserInterfaceThreadHandle = NULL;
	InitLogFile(Client.LogFilePtr);
}

void HandleClient() {
	InitWsaData();
	Client.Socket = CreateOneSocket();
	ConnectToServer();
	CreateThreads();
}

void ConnectToServer() {
	int ConnectReturnValue;
	char ConnectMessage[MESSAGE_LENGTH];

	Client.SocketService.sin_family = AF_INET;
	Client.SocketService.sin_addr.s_addr = inet_addr(Client.ServerIP);
	Client.SocketService.sin_port = htons(Client.ServerPortNum);
	
	ConnectReturnValue = connect(Client.Socket, (SOCKADDR*)&Client.SocketService, sizeof(Client.SocketService));
	if (ConnectReturnValue == SOCKET_ERROR) {
		sprintf(ConnectMessage, "Failed connecting to server on %s:%d. Exiting.\n", Client.ServerIP, Client.ServerPortNum);
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, ConnectMessage);
		CloseSocketAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}
	sprintf(ConnectMessage, "Connected to server on %s:%d.\n", Client.ServerIP, Client.ServerPortNum);
	OutputMessageToWindowAndLogFile(Client.LogFilePtr, ConnectMessage);
}

void CloseSocketAndThreads() { // todo
	CloseOneSocket(Client.Socket);
	CloseOneThreadHandle(Client.SendThreadHandle, Client.LogFilePtr);
	CloseOneThreadHandle(Client.ReceiveThreadHandle, Client.LogFilePtr);
	CloseOneThreadHandle(Client.UserInterfaceThreadHandle, Client.LogFilePtr);
	CloseWsaData(Client.LogFilePtr);
}