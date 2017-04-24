/*----------------------------------------------------------------
* passivesock - allocate & bind a server socket using TCP
*----------------------------------------------------------------
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>

SOCKET passivesock(char *service, char *protocol) {
	// [IN] service : char array containing service type as name (eg. ECHO) or port number (eg. 7)
	// [IN] protocl : char array containing protocol type (either "tcp" or "udp")
	// Return value : SOCKET handle of server-type socket.

	//	NOTE: This function requires the iostream and winsock.h header files
	//        as well as the ws2_32.lib library file.

	servent     *pse;		    // pointer to service information entry
	sockaddr_in  socketInfo;	// an Internet endpoint address
	SOCKET       s;			    // socket descriptor

								// Use protocol to allocate TCP or UDP type socket
	if (_stricmp(protocol, "tcp") == 0) {
		s = socket(AF_INET, SOCK_STREAM, 0);
	}
	else if (_stricmp(protocol, "udp") == 0) {
		s = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else {
		s = INVALID_SOCKET;
	}
	if (s < 0 || s == INVALID_SOCKET) {
		std::cout << "Cannot create socket.  Error Code = " << WSAGetLastError() << std::endl;
		return INVALID_SOCKET;
	}

	memset(&socketInfo, 0, sizeof(socketInfo));	// zero the entire sockaddr_in struct

	socketInfo.sin_family = AF_INET;			// Set the address family to AF_INET (= Internet)
	socketInfo.sin_addr.s_addr = INADDR_ANY;	// Set IP Address to INADDR_ANY constant

												/* Map service name (or number) to port number */
	pse = getservbyname(service, protocol); // Search for matching service name
	if (pse != NULL) {                  // If found, retrieve associated port number
		socketInfo.sin_port = (u_short)pse->s_port;
	}
	else {                              // If not found, treat service as an integer
		short port = atoi(service);
		if (port > 0) {
			socketInfo.sin_port = htons(port);
		}
		else {
			std::cout << "Invalid service or port number:" << service << std::endl;
			return INVALID_SOCKET;
		}
	}

	/* Bind the socket */
	if (bind(s, (LPSOCKADDR)&socketInfo, sizeof(SOCKADDR)) < 0) {
		std::cout << "Cannot bind socket to port: " << service << "  (Error Code = " << WSAGetLastError() << ")" << std::endl;
		return INVALID_SOCKET;
	}

	if (_stricmp(protocol, "tcp") == 0) { // Call listen function ONLY if protocol is TCP
		if (listen(s, 5) < 0) {
			std::cout << "Cannot listen on port: " << service << "(Error Code = " << WSAGetLastError() << ")" << std::endl;
			return INVALID_SOCKET;
		}
	}

	return s;
}