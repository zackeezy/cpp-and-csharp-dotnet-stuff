/*------------------------------------------------------------------
 * connectsock - allocate & connect a client socket using TCP or UDP
 *------------------------------------------------------------------*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>

SOCKET connectsock(char *host, char *service, char *protocol) {
	// Parameter List:
	// [IN] host    : String containing host as either a DNS name (eg. "www.harding.edu") or an IP address (e.g. "162.242.214.217")
	// [IN] service : String containing service type as name (eg. "ECHO") or port number (eg. "7")
	// [IN] protocol: String containing protocol type (either "tcp" or "udp")
	// Return value : SOCKET handle, s, of connected socket.

	//	NOTES: 1. This function requires the iostream and winsock2.h header files as well as the ws2_32.lib library file.
	//         2. The host and service parameters are not used for UDP sockets.  Empty strings may be used if protocol = "UDP".

	struct hostent  *phe;			// pointer to host information entry     
	struct servent  *pse;			// pointer to service information entry  
	struct sockaddr_in clientInfo;	// an Intenet endpoint address  (socket information)        
	SOCKET			s;				// socket handle                        

	// Allocate and connect socket - type depends upon protocol 
	// If using TCP ...
	if (_stricmp(protocol, "tcp") == 0) {
		/* Create a TCP socket */
		s = socket(AF_INET, SOCK_STREAM, 0);
		if (s < 0 || s == INVALID_SOCKET) {
			std::cout << "Cannot create socket.  Error Code = " << WSAGetLastError() << std::endl;
			return INVALID_SOCKET;
		}

		memset(&clientInfo, 0, sizeof(clientInfo));		// zero the sockaddr_in struct

		// Set the address family to AF_INET (= Internet) 
		clientInfo.sin_family = AF_INET;

		// Map host name to remote IP address, allowing for dotted quad 
		long ihost;	   // 32-bit IP address number in Network Byte Order (big endian) 
		ihost = inet_addr(host);	// 1st try to convert from "dotted decimal notation" 
		if (ihost == INADDR_NONE) {	// if that doesn't work, send request to DNS 
			phe = gethostbyname(host);
			if (phe != NULL) { // If phe isn't NULL, DNS returned a matching IP address!
				memmove(&ihost, phe->h_addr, phe->h_length);
			}
			else {
				std::cout << "Invalid host" << std::endl;
				return INVALID_SOCKET;
			}
		}
		clientInfo.sin_addr.s_addr = ihost;	// Assign integer IP address to field in clientInfo 

		// Map service name (or number) to port number 
		pse = getservbyname(service, protocol);	// Search for matching service name
		if (pse != NULL) {						// If found, retrieve associated port number
			clientInfo.sin_port = (u_short)pse->s_port;
		}
		else {									// If not found, treat service as an integer
			short port = atoi(service);
			if (port > 0) {
				clientInfo.sin_port = htons(port);
			}
			else {
				std::cout << "Invalid service request" << std::endl;
				return INVALID_SOCKET;
			}
		}

		// Connect the TCP socket
		int status = connect(s, (LPSOCKADDR)&clientInfo, sizeof(SOCKADDR));
		if (status == SOCKET_ERROR) {
			std::cout << "Remote host/service not found - or connection refused.  Error Code = " << WSAGetLastError() << std::endl;
			return INVALID_SOCKET;
		}
	}

	// If using UDP ...
	else if (_stricmp(protocol, "udp") == 0) {
		// Create a UDP socket (Nothing else to do!!)
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s < 0 || s == INVALID_SOCKET) {
			std::cout << "Cannot create socket.  Error Code = " << WSAGetLastError() << std::endl;
			return INVALID_SOCKET;
		}
	}

	// This function doesn't support anything other than TCP or UDP
	else {
		std::cout << "Invalid Protocol" << std::endl;
		return INVALID_SOCKET;
	}

	return s;
}