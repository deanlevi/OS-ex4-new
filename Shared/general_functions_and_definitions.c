#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>

#include "general_functions_and_definitions.h"

void InitWsaData();
void InitLogFile(char *LogFilePathPtr);
void OutputMessageToWindowAndLogFile(char *LogFilePathPtr, char *MessageToWrite);
void WriteToLogFile(char *LogFilePathPtr, char *MessageToWrite);
HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id,
						  char *LogFilePathPtr);
void CloseOneThreadHandle(HANDLE HandleToClose, char *LogFilePathPtr);
void CloseWsaData(char *LogFilePathPtr);

void InitWsaData() {
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR) {
		printf("Error %ld at WSAStartup().\nExiting...\n", StartupRes);
		// Tell the user that we could not find a usable WinSock DLL.
		exit(ERROR_CODE); // todo handle error
	}
}

void InitLogFile(char *LogFilePathPtr) {
	FILE *LogFilePointer = fopen(LogFilePathPtr, "w");
	if (LogFilePointer == NULL) {
		printf("Couldn't open log file.\n");
		exit(ERROR_CODE);
	}
	fclose(LogFilePointer);
}

void OutputMessageToWindowAndLogFile(char *LogFilePathPtr, char *MessageToWrite) {
	printf("%s", MessageToWrite);
	WriteToLogFile(LogFilePathPtr, MessageToWrite); // todo
}

void WriteToLogFile(char *LogFilePathPtr, char *MessageToWrite) { // todo check all calls to this function
	FILE *LogFilePointer = fopen(LogFilePathPtr, "a");
	if (LogFilePointer == NULL) {
		printf("Couldn't open log file.\n");
		exit(ERROR_CODE);
	}
	fputs(MessageToWrite, LogFilePointer);
	fclose(LogFilePointer);
}

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id,
						  char *LogFilePathPtr) {
	HANDLE thread_handle;

	if (NULL == p_start_routine) {
		OutputMessageToWindowAndLogFile(LogFilePathPtr, "Error when creating a thread. Received null pointer.\n");
		CloseSocketsAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}

	if (NULL == p_thread_id) {
		OutputMessageToWindowAndLogFile(LogFilePathPtr, "Error when creating a thread. Received null pointer.\n");
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
		OutputMessageToWindowAndLogFile(LogFilePathPtr, "Couldn't create thread.\n");
		CloseSocketsAndThreads(); // todo check if add function to handle error
		exit(ERROR_CODE);
	}

	return thread_handle;
}

void CloseOneThreadHandle(HANDLE HandleToClose, char *LogFilePathPtr) {
	DWORD ret_val;

	ret_val = CloseHandle(HandleToClose);
	if (FALSE == ret_val) {
		OutputMessageToWindowAndLogFile(LogFilePathPtr, "Error when closing threads.\n");
		exit(ERROR_CODE);
	}
}

void CloseWsaData(char *LogFilePathPtr) {
	if (WSACleanup() == SOCKET_ERROR) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		OutputMessageToWindowAndLogFile(ErrorMessage, LogFilePathPtr);
		exit(ERROR_CODE);
	}
}