#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>

#include "client.h"

void InitClient(char *argv[]);
void HandleClient();
void ConnectToServer();
void CreateThreadsAndSemaphores();
void WINAPI SendThread();
void HandleNewUserRequest();
void WINAPI ReceiveThread();
void HandleNewUserAccept();
void ParseNewUserAccept(char *ReceivedData);
void HandleReceivedData(char *ReceivedData);
void WINAPI UserInterfaceThread();
void CloseSocketAndThreads();


void InitClient(char *argv[]) {
	Client.Socket = INVALID_SOCKET;
	Client.LogFilePtr = argv[1];
	Client.ServerIP = argv[2];
	Client.ServerPortNum = atoi(argv[3]);
	Client.UserName = argv[4];

	int ThreadIndex = 0;
	for (; ThreadIndex < NUMBER_OF_THREADS_TO_HANDLE_CLIENT; ThreadIndex++) {
		Client.ThreadHandles[ThreadIndex] = NULL;
	}
	Client.ReceiveDataSemaphore = NULL;
	Client.PlayerType = None;
	InitLogFile(Client.LogFilePtr);
}

void HandleClient() {
	DWORD wait_code;

	InitWsaData();
	Client.Socket = CreateOneSocket();
	if (Client.Socket == INVALID_SOCKET) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: HandleClient failed to create socket.\nError Number is %d\n", WSAGetLastError());
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, ErrorMessage);
		exit(ERROR_CODE);
	}

	ConnectToServer();
	CreateThreadsAndSemaphores();

	wait_code = WaitForMultipleObjects(NUMBER_OF_THREADS_TO_HANDLE_CLIENT, Client.ThreadHandles, TRUE, INFINITE); // todo check INFINITE
	if (WAIT_OBJECT_0 != wait_code) {
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Custom message: Error when waiting for program to end.\n");
		CloseSocketAndThreads(); // todo add/check print
		exit(ERROR_CODE);
	}
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

void CreateThreadsAndSemaphores() {
	Client.ThreadHandles[0] = CreateThreadSimple((LPTHREAD_START_ROUTINE)SendThread,
		NULL,
		&Client.ThreadIDs[0],
		Client.LogFilePtr);

	Client.ThreadHandles[1] = CreateThreadSimple((LPTHREAD_START_ROUTINE)ReceiveThread,
		NULL,
		&Client.ThreadIDs[1],
		Client.LogFilePtr);

	Client.ThreadHandles[2] = CreateThreadSimple((LPTHREAD_START_ROUTINE)UserInterfaceThread,
		NULL,
		&Client.ThreadIDs[2],
		Client.LogFilePtr);

	if (Client.ThreadHandles[0] == NULL || Client.ThreadHandles[1] == NULL || Client.ThreadHandles[2] == NULL) {
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Custom message: CreateThreadsAndSemaphores failed to create threads.\n");
		CloseSocketAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}

	Client.ReceiveDataSemaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		0,		/* Initial Count - not signaled */
		1,		/* Maximum Count */
		NULL);	/* un-named */

	if (Client.ReceiveDataSemaphore == NULL) {
		OutputMessageToWindowAndLogFile(Client.LogFilePtr,
			"Custom message: CreateThreadsAndSemaphores - Error when creating SendDataSemaphore semaphore.\n");
		CloseSocketAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}
}

void WINAPI SendThread() {
	HandleNewUserRequest();
	while (TRUE) {
		break; // todo remove
	}
}

void HandleNewUserRequest() {
	char NewUserRequest[MESSAGE_LENGTH];
	int SendDataToServerReturnValue;
	sprintf(NewUserRequest, "NEW_USER_REQUEST:%s\n", Client.UserName);
	SendDataToServerReturnValue = SendData(Client.Socket, NewUserRequest, Client.LogFilePtr); // todo check if add mutex
	if (SendDataToServerReturnValue == ERROR_CODE) {
		CloseSocketAndThreads();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent NEW_USER_REQUEST to Server. UserName %s.\n", Client.UserName);
	OutputMessageToWindowAndLogFile(Client.LogFilePtr, TempMessage);

	if (ReleaseOneSemaphore(Client.ReceiveDataSemaphore) == FALSE) { // to receive NEW_USER_ACCEPTED
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Custom message: SendThread - failed to release receive semaphore.\n");
		CloseSocketAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}
}

void WINAPI ReceiveThread() {
	HandleNewUserAccept();

	while (TRUE) {
		char *ReceivedData = ReceiveData(Client.Socket, Client.LogFilePtr);
		if (ReceivedData == NULL) {
			CloseSocketAndThreads();
			exit(ERROR_CODE);
		}
		HandleReceivedData(ReceivedData);
	}
}

void HandleNewUserAccept() {
	/*DWORD wait_code;
		wait_code = WaitForSingleObject(Client.ReceiveDataSemaphore, INFINITE); // wait for signal // todo check if needed
	if (WAIT_OBJECT_0 != wait_code) {
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Custom message: Error when waiting for ReceiveData semaphore.\n");
		CloseSocketAndThreads();
		exit(ERROR_CODE);
	}*/
	char *ReceivedData = ReceiveData(Client.Socket, Client.LogFilePtr);
	if (ReceivedData == NULL) {
		CloseSocketAndThreads();
		exit(ERROR_CODE);
	}
	ParseNewUserAccept(ReceivedData);
	char TempMessage[MESSAGE_LENGTH];
	char PlayerType = Client.PlayerType == X ? 'x' : 'o';
	sprintf(TempMessage, "Custom message: %s received NEW_USER_ACCEPTED from Server. PlayerType is %c.\n", Client.UserName, PlayerType);
	OutputMessageToWindowAndLogFile(Client.LogFilePtr, TempMessage);
}

void ParseNewUserAccept(char *ReceivedData) {
	int NewUserMessageOffset = 17; // size of "NEW_USER_DECLINED" / "NEW_USER_ACCEPTED"
	if (strncmp(ReceivedData, "NEW_USER_DECLINED", NewUserMessageOffset) == 0 ||
		strncmp(ReceivedData, "NEW_USER_ACCEPTED", NewUserMessageOffset) != 0) {
		if (strncmp(ReceivedData, "NEW_USER_DECLINED", NewUserMessageOffset) == 0) {
			OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Request to join was refused.\n");
		}
		else {
			OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Custom message: Got unexpected answer from Server. Exiting...\n");
		}
		free(ReceivedData);
		CloseSocketAndThreads();
		exit(ERROR_CODE);
	}
	NewUserMessageOffset += 1;
	if (strncmp(ReceivedData + NewUserMessageOffset, "x", 1) == 0) {
		Client.PlayerType = X;
	} else if (strncmp(ReceivedData + NewUserMessageOffset, "o", 1) == 0) {
		Client.PlayerType = O;
	}
	else {
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Custom message: Got unexpected answer from Server. Exiting...\n");
		free(ReceivedData);
		CloseSocketAndThreads();
		exit(ERROR_CODE);
	}
	// todo check if need to parse player number. 1/2.
	free(ReceivedData);
}

void HandleReceivedData(char *ReceivedData) {

}

void WINAPI UserInterfaceThread() {
	while (TRUE) {
		break; // todo remove
	}
}

void CloseSocketAndThreads() { // todo
	CloseOneSocket(Client.Socket, Client.LogFilePtr);
	int ThreadIndex = 0;
	for (; ThreadIndex < NUMBER_OF_THREADS_TO_HANDLE_CLIENT; ThreadIndex++) {
		CloseOneThreadHandle(Client.ThreadHandles[ThreadIndex], Client.LogFilePtr);
	}
	CloseWsaData(Client.LogFilePtr);
}