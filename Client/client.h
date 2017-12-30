#ifndef CLIENT_H
#define CLIENT_H

#include "../Shared/socket.h" // todo

#define NUMBER_OF_THREADS_TO_HANDLE_CLIENT 3

typedef struct ClientProperties {
	SOCKET Socket; // the socket that is connecting to the server
	SOCKADDR_IN SocketService;
	char *LogFilePtr; // path to log file
	char *ServerIP; // server's IP
	int ServerPortNum; // server's port num
	char *UserName; // todo why need to assume <= 30 ?
	HANDLE ThreadHandles[NUMBER_OF_THREADS_TO_HANDLE_CLIENT]; // one for send, one for receive, and one for user interface
	DWORD ThreadIDs[NUMBER_OF_THREADS_TO_HANDLE_CLIENT]; // thread ids for the above thread handles

	HANDLE UserInterfaceSemaphore; // semaphore to block user interface thread until connection is established and user is accepted

	PlayerType PlayerType;
}ClientProperties;

ClientProperties Client;

void InitClient(char *argv[]);
void HandleClient();
void CloseSocketAndThreads();

#endif