#ifndef CLIENT_H
#define CLIENT_H

#include "../Shared/socket.h" // todo

typedef struct ClientProperties {
	SOCKET Socket; // the socket that is connecting to the server
	SOCKADDR_IN SocketService;
	char *LogFilePtr; // path to log file
	char *ServerIP; // server's IP
	int ServerPortNum; // server's port num
	char *UserName; // todo why need to assume <= 30 ?
	HANDLE SendThreadHandle; // thread handle to send data to server
	DWORD SendThreadID; // thread id for the above thread handle
	HANDLE ReceiveThreadHandle; // thread handle to receive data to server
	DWORD ReceiveThreadID; // thread id for the above thread handle
	HANDLE UserInterfaceThreadHandle; // thread handle to communicate with client user
	DWORD UserInterfaceThreadID; // thread id for the above thread handle
	
}ClientProperties;

ClientProperties Client;

void InitClient(char *argv[]);
void HandleClient();
void CloseSocketAndThreads();

#endif