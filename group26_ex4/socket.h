#ifndef SOCKET_H
#define SOCKET_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // todo move to shared
#endif

#include <WinSock2.h> // todo move to shared
#include <WS2tcpip.h> // todo move to shared

#pragma comment(lib, "Ws2_32.lib") // todo move to shared

#define ERROR_CODE (int)(-1) // todo move to shared

SOCKET CreateOneSocket(); // todo fix function
void CloseOneSocket(SOCKET Socket);

#endif