#include "TicTacToe.h"
#include <WinSock2.h>
#include <iostream>

// getServers.cpp
//  Given a socket, a broadcast address and a port number, this function returns the number
//  of servers as well as an array of structs.  Each struct contains the name, IP_Address and 
//  port number of a remote server.

//    IN parameters:
//    SOCKET s                = Allocate UDP socket handle that can be used for communcations
//    char *broadcastAddress  = The broadcast address that should be used for current LAN
//    char *broadcastPort     = The port that should be used for the broadcast

//    OUT parameters:
//    ServerStruct server[]   = An array that contains the structs.  The members of each struct
//                              contain the name, IP Address and port number of remote server.
//    numServers              = Number of elements in the server[] array

//    Function return value   = Number of remote servers (size of server[] array)

bool strcmp_substr(char* lVal, char* rVal, int len) {
	for (int i = 0; i < len; i++) {
		if (lVal[i] != rVal[i] || lVal[i] == '\0' || rVal[i] == '\0') {
			return false;
		}
	}
	return true;
}

std::string c_substr(char* c_str, int begin, int end) {
	std::string retVal = c_str;
	/*int x = 0;
	for (int i = begin; i < end; ++i) {
	retVal += c_str[i];
	}*/
	retVal = retVal.substr(5);
	return retVal;
}

int strsize(char* in) {
	int i = 0;
	for (; in[i] != '\0'; i++) {}
	return i;
}

int getServers(SOCKET s, char *broadcastAddress, char *broadcastPort, ServerStruct server[])
{
	int numServers = 0;

	int len3 = UDP_send(s, TicTacToe_QUERY, sizeof(TicTacToe_QUERY), broadcastAddress, broadcastPort);

	int status = wait(s, 2, 0);
	if (status > 0) {
		char resp[MAX_RECV_BUF];

		int len4 = UDP_recv(s, resp, MAX_RECV_BUF, broadcastAddress, broadcastPort);

		while (status > 0 && len4 > 0) {
			if (strcmp_substr(resp, TicTacToe_NAME, 5)) {
				server[numServers].name = c_substr(resp, 5, strsize(resp) - 1);
				server[numServers].host = broadcastAddress;
				server[numServers++].port = broadcastPort;
			}

			status = wait(s, 2, 0);
			if (status > 0)
				len4 = UDP_recv(s, resp, MAX_RECV_BUF, broadcastAddress, broadcastPort);
		}
	}
	return numServers;
}