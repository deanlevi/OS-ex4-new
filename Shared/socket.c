#include <stdio.h> // todo remove if removing printf

#include "socket.h"

SOCKET CreateOneSocket();
void CloseOneSocket(SOCKET Socket);


SOCKET CreateOneSocket() { // todo fix function
	SOCKET NewSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (NewSocket == INVALID_SOCKET) {
		printf("CreateOneSocket failed to create Socket.\n Error Number is %d\n" , WSAGetLastError());
		exit(ERROR_CODE); // todo handle error and check print
	}
	return NewSocket;
}

void CloseOneSocket(SOCKET Socket) {
	int CloseSocketReturnValue;
	if (Socket != INVALID_SOCKET) {
		CloseSocketReturnValue = closesocket(Socket);
		if (CloseSocketReturnValue == SOCKET_ERROR) {
			printf("CloseOneSocket failed to close socket.\n Error Number is %d\n", WSAGetLastError());
			exit(ERROR_CODE); // todo handle error and check print
		}
	}
}