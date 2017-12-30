#ifndef SERVER_H
#define SERVER_H

#include "../Shared/socket.h" // todo

#define SERVER_ADDRESS_STR "127.0.0.1" // todo move to shared
#define NUMBER_OF_CLIENTS 2
#define NUMBER_OF_GAMES 1 // todo
#define BINDING_SUCCEEDED 0
#define LISTEN_SUCCEEDED 0
#define USER_NAME_LENGTH 31

typedef struct _Player {
	char UserName[USER_NAME_LENGTH];
	PlayerType PlayerType; // X - first player, O - second player
}Player;

typedef struct _ServerProperties {
	SOCKET ListeningSocket; // the socket that is listening to client connections
	SOCKADDR_IN ListeningSocketService;
	SOCKET ClientsSockets[NUMBER_OF_CLIENTS]; // the sockets that are connected to each client
	int ClientIndex[NUMBER_OF_CLIENTS]; // to send to thread function
	char *LogFilePtr; // path to log file
	int PortNum; // server's port num
	HANDLE ClientsThreadHandle[NUMBER_OF_CLIENTS]; // thread handle to each client
	DWORD ClientsThreadID[NUMBER_OF_CLIENTS]; // thread id to each client
	Player Players[NUMBER_OF_CLIENTS];

}ServerProperties;

ServerProperties Server;

void InitServer(char *argv[]);
void HandleServer();
void CloseSocketsAndThreads();

#endif