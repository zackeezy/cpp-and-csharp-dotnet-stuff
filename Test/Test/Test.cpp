// This is the main DLL file.

#include "stdafx.h"
#include <WinSock2.h>
#include <string>
#include <comdef.h>

#include "Test.h"


namespace Test {
	Boolean Networking::WSAInit() {
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			return 0;
		}
		return 1;
	}

	Int32 Networking::WSACleanupWrapper() {
		closesocket(s);
		return WSACleanup();
	}
	
	Int32 Networking::wait(Int32 seconds, Int32 msec) {
		// Parameter List:
		// [IN] s		: SOCKET handle of connected socket.
		// [IN] seconds : Number of seconds to wait for incoming traffic
		// [IN] msec	: Number of milliseconds to wait
		// Return value : 1 = Data is waiting in TCP/UDP buffer; 0 = Timeout or error detected

		// Given a socket and an amount of time (in seconds + milliseconds), this function
		// will check to see if any data is waiting to be read from the socket.  The function
		// will wait for the alloted amount of time.  If no data arrives at the socket within
		// the alloted amount of time, the function will exit and return the value 0.
		// CAUTION:  If the remote machine associated with a TCP socket has closed the connection,
		//           this function may still return 1.  This is because the socket is in the
		//           "close pending" state.  A subsequent call to recv() or recvfrom() will 
		//           immediately return 0 (if connection was closed gracefully) or SOCKET_ERROR
		//           if the connection was aborted.  Thus, it is important to check the return
		//           value of the recv() function that follows a call to this wait() function.

		// Note: This function requires the winsock.h header file and wsock32.lib library.

		int    stat;					// Status of waiting socket
		struct timeval stTimeOut;		// Used to set timeout value
		fd_set stReadFDS;				// "File Descriptor Set" for select() call
		fd_set stXcptFDS;				// "File Descriptor Set" for exception handling

										// Set TimeOut value
		stTimeOut.tv_sec = seconds;	// Number of seconds
		stTimeOut.tv_usec = msec;		//   + msec milliseconds to wait

		FD_ZERO(&stReadFDS);			// Zero the set of "Read access" sockets
		FD_ZERO(&stXcptFDS);			// Zero the set of "Exception" sockets
		FD_SET(s, &stReadFDS);			// Add "s" to the set of "Read access" sockets
		FD_SET(s, &stXcptFDS);			// Add "s" to the set of "Exception" sockets

										// Check to see if Read access is enabled for socket "s"
		stat = select(-1, &stReadFDS, NULL, &stXcptFDS, &stTimeOut);
		if (stat == SOCKET_ERROR) {
			//std::cout << std::endl << "wait() function failed - Error Code = " << WSAGetLastError() << std::endl;
			stat = 0;
		}
		else if (stat > 0) {
			if (FD_ISSET(s, &stXcptFDS)) {	// Some error was detected or OOB data
				stat = 0;
			}
			if (!FD_ISSET(s, &stReadFDS)) {	// No incoming data!? (check just in case)
				stat = 0;
			}
		}

		// stat will be 0 if stTimeOut exceeded or an error was detected; otherwise stat = 1, 
		// which indicates the socket has pending data.
		return stat;
	}

	Int32 Networking::UDP_send(String ^buf, Int32 numBytesToSend, String ^host, String ^service)
	{	// Parameter List:
		// [IN] s      : SOCKET handle (removed for port to C#)
		// [IN] buf    : Address of NULL-terminated char array containing bytes to be sent to host
		// [IN] numBytesToSend: Integer containing the number of bytes in buf that should be sent
		//                       Caution: This integer should be <= the size of the buf char array!
		// [IN] host   : Address of NULL-terminated char array containing domain name of remote host or IP Address
		//               Broadcast-type IP Address (such as 10.1.255.255) is supported.
		// [IN] service: Address of NULL-terminated char array containing service name or port number to use for transmission
		// Return Value: Number of bytes actually sent.

		//	NOTE: This function requires the <iostream> and WinSock2.h header files
		//        as well as the ws2_32.lib library file.

		struct hostent  *phe;   /* pointer to host information entry     */
		struct servent	*pse;	/* pointer to service information entry  */
		sockaddr_in		sin;	/* an Intenet endpoint address           */
		long			ihost;	/* 32-bit IP address number              */
		int				len;	/* Number of bytes actually sent         */

		if (s < 0 || s == INVALID_SOCKET) {
			Console::WriteLine("Problem with UDP_send().  Invalid socket handle.");
			len = -1;
		}
		else {
			memset(&sin, 0, sizeof(sin));  /* zero the sockaddr_in struct */

										   /* Set the address family to AF_INET (= Internet) */
			sin.sin_family = AF_INET;

			/* Map service name (or number) to port number */

			IntPtr ptrToNativeString = Marshal::StringToHGlobalAnsi(service);
			char* serviceString = static_cast<char*>(ptrToNativeString.ToPointer());
			if (pse = getservbyname(serviceString, "UDP"))
				sin.sin_port = pse->s_port;
			else if ((sin.sin_port = htons((u_short)atoi(serviceString))) == 0) {
				Console::WriteLine("Invalid service request.");
				return INVALID_SOCKET;
			}

			/* Map host name to IP address, allowing for dotted quad */
			ptrToNativeString = Marshal::StringToHGlobalAnsi(host);
			char* hostString = static_cast<char*>(ptrToNativeString.ToPointer());
			ihost = inet_addr(hostString);
			if (ihost == INADDR_NONE) {
				phe = gethostbyname(hostString);
				if (phe)
					memmove(&ihost, phe->h_addr, phe->h_length);
				else {
					Console::WriteLine("Invalid host");
					return INVALID_SOCKET;
				}
			}
			sin.sin_addr.s_addr = ihost;
			ptrToNativeString = Marshal::StringToHGlobalAnsi(buf);
			char* nativeString = static_cast<char*>(ptrToNativeString.ToPointer());
			len = sendto(s, nativeString, numBytesToSend, 0, (LPSOCKADDR)&sin, sizeof(sin));
			if (len < 0) {
				Console::WriteLine("Problem with sendto() function call within UDP_send().  Error Code = " + WSAGetLastError());
			}
			else if (len == 0) {
				Console::WriteLine("Warning: Sending UDP datagram with 0 bytes.");
			}
		}

		return len;
	}
	
	Int32 Networking::UDP_recv(String ^buf, Int32 maxsize, String ^host, String ^port)
	{	// Parameter List:
		// [IN] s      : SOCKET handle
		// [OUT] buf   : Address of NULL-terminated char array that will contain the data received.
		// [IN] maxsize: Integer containing the maximum number of bytes that buf can hold
		// [OUT] host  : Address of NULL-terminated char array containing IP Address of machine that sent the data
		// [OUT] port  : Address of NULL-terminated char array containing port number on remote host that sent the data
		// Return Value: Number of bytes actually received.

		// NOTES: 1. This function requires WinSock2.h and iostream header files
		//             as well as the ws2_32.lib library file.
		//        2. This function will block until data is received from some other machine.

		sockaddr_in sin;						/* an Intenet endpoint address */
		int         sin_size = sizeof(sin);		/* Needed for sixth parameter in recvfrom function call */
		int         len;						/* Holds return value for recvfrom function call */

		if (s < 0 || s == INVALID_SOCKET) {
			Console::WriteLine("Problem with UDP_recv().  Invalid socket handle.");
			len = -1;
		}
		else {
			memset(&sin, 0, sizeof(sin));				/* zero the sockaddr_in struct */

														/* Wait for and read incoming data from ANY remote host/port */
			char* bufBuf = new char[maxsize];
			len = recvfrom(s, bufBuf, maxsize, 0, (LPSOCKADDR)&sin, &sin_size);
			// Make sure the characters in buf[] are null-terminated
			if (len > 0) {
				if (bufBuf[len - 1] != '\0') {
					if (len < maxsize)
						bufBuf[len] = '\0';
					else
						bufBuf[maxsize - 1] = '\0';
				}
			}
			else {
				bufBuf[0] = '\0';
			}

			buf = gcnew String(bufBuf);
			delete bufBuf;

			if (len < 0 || len > maxsize) {
				int errorCode = WSAGetLastError();
				if (errorCode != WSAECONNRESET) {
					//std::cout << std::endl << "Problem with recvfrom() function call within UDP_recv().  Error Code = " << errorCode << std::endl;
					Console::WriteLine("Problem with recvfrom() function call within UDP_recv().Error Code = " + errorCode);
				}
			}
			else {
				// Find the sender's IP address and port #
				char hostBuf[MAX_IP_ADDRESS_LEN];
				strcpy_s(hostBuf, MAX_IP_ADDRESS_LEN, inet_ntoa(sin.sin_addr));
				host = gcnew String(hostBuf);
				char portBuf[MAX_PORT_LEN];
				_itoa_s(ntohs(sin.sin_port), portBuf, MAX_PORT_LEN, 10);
				port = gcnew String(portBuf);
			}
		}
		return len;
	}

	Boolean Networking::initPassiveSock(String ^service, String ^protocol) {
		// [IN] service : char array containing service type as name (eg. ECHO) or port number (eg. 7)
		// [IN] protocl : char array containing protocol type (either "tcp" or "udp")
		// Return value : SOCKET handle of server-type socket.

		//	NOTE: This function requires the iostream and winsock.h header files
		//        as well as the ws2_32.lib library file.

		servent     *pse;		    // pointer to service information entry
		sockaddr_in  socketInfo;	// an Internet endpoint address
		
		// Use protocol to allocate TCP or UDP type socket
		if (protocol->Equals("tcp")) {
			s = socket(AF_INET, SOCK_STREAM, 0);
		}
		else if (protocol->Equals("udp")) {
			s = socket(AF_INET, SOCK_DGRAM, 0);
		}
		else {
			s = INVALID_SOCKET;
		}
		if (s < 0 || s == INVALID_SOCKET) {
			Console::WriteLine("Cannot create socket.  Error Code = " + WSAGetLastError());
			return false;
		}

		memset(&socketInfo, 0, sizeof(socketInfo));	// zero the entire sockaddr_in struct

		socketInfo.sin_family = AF_INET;			// Set the address family to AF_INET (= Internet)
		socketInfo.sin_addr.s_addr = INADDR_ANY;	// Set IP Address to INADDR_ANY constant

		IntPtr ptrToNativeString = Marshal::StringToHGlobalAnsi(service);
		char* serviceString = static_cast<char*>(ptrToNativeString.ToPointer());
		ptrToNativeString = Marshal::StringToHGlobalAnsi(protocol);
		char* protocolString = static_cast<char*>(ptrToNativeString.ToPointer());
													/* Map service name (or number) to port number */
		pse = getservbyname(serviceString, protocolString); // Search for matching service name
		if (pse != NULL) {                  // If found, retrieve associated port number
			socketInfo.sin_port = (u_short)pse->s_port;
		}
		else {                              // If not found, treat service as an integer
			Int32 port = Convert::ToInt32(service);
			if (port > 0) {
				socketInfo.sin_port = htons(port);
			}
			else {
				const char * serviceTemp = serviceString;
				String ^s = gcnew String(serviceString);
				Console::WriteLine("Invalid service or port number:" + s);
				return false;
			}
		}

		/* Bind the socket */
		if (bind(s, (LPSOCKADDR)&socketInfo, sizeof(SOCKADDR)) < 0) {
			String ^s = gcnew String(serviceString);
			Console::WriteLine("Cannot listen on port: " + s + "(Error Code = " + WSAGetLastError().ToString() + ")");
			return false;
		}

		if (protocol->Equals("tcp")) { // Call listen function ONLY if protocol is TCP
			if (listen(s, 5) < 0) {
				String ^s = gcnew String(serviceString);
				Console::WriteLine("Cannot listen on port: " + s + "(Error Code = " + WSAGetLastError().ToString() + ")");
				return false;
			}
		}

		return true;
	}

	String ^Networking::getBroadcastAddress()
	{
		// NOTE: This code only returns the broadcast
		//  address for the first adapter found.

		// Declare and initialize variables
#define MAXSIZE 16
		static char Address[MAXSIZE] = "";
		unsigned long IPAddress = 0;
		unsigned long subnetMask = 0;
		unsigned long broadcastAddress = 0;

		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;
		DWORD dwRetVal = 0;
		ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

		// Make an initial call to GetAdaptersInfo to get
		// the necessary size into the ulOutBufLen variable
		pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, sizeof(IP_ADAPTER_INFO));
		if (pAdapterInfo == NULL) {
			Console::WriteLine("Error allocating memory needed to call GetAdaptersinfo");
			return NULL.ToString();
		}
		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
			// Release initial allocation of heap memory and request more appropriately sized memory
			HeapFree(GetProcessHeap(), 0, pAdapterInfo);
			pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);
			if (pAdapterInfo == NULL) {
				Console::WriteLine("Error allocating memory needed to call GetAdaptersinfo");
				return NULL.ToString();
			}
		}

		if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
			pAdapter = pAdapterInfo;
			if (pAdapter) {
				IPAddress = inet_addr(pAdapter->IpAddressList.IpAddress.String);
				subnetMask = inet_addr(pAdapter->IpAddressList.IpMask.String);
				unsigned long BcastMask = subnetMask ^ 0xffffffff;
				broadcastAddress = IPAddress | BcastMask;
				in_addr ip_addr;
				ip_addr.S_un.S_addr = broadcastAddress;
				strcpy_s(Address, MAXSIZE, inet_ntoa(ip_addr));
			}
		}
		else {
			std::cout << "GetAdaptersInfo failed with error: " << dwRetVal << std::endl;
		}
		if (pAdapterInfo)
			HeapFree(GetProcessHeap(), 0, pAdapterInfo);

		String ^s = gcnew String(Address);
		return s;
	}

	Boolean Networking::initSock(String ^host, String ^service, String ^protocol) {
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

										// Allocate and connect socket - type depends upon protocol 
										// If using TCP ...
		if (protocol->Equals("tcp")) {
			/* Create a TCP socket */
			s = socket(AF_INET, SOCK_STREAM, 0);
			if (s < 0 || s == INVALID_SOCKET) {
				Console::WriteLine("Cannot create socket.  Error Code = " + WSAGetLastError());
				return false;
			}

			memset(&clientInfo, 0, sizeof(clientInfo));		// zero the sockaddr_in struct

															// Set the address family to AF_INET (= Internet) 
			clientInfo.sin_family = AF_INET;

			IntPtr ptrToNativeString = Marshal::StringToHGlobalAnsi(host);
			char* hostString = static_cast<char*>(ptrToNativeString.ToPointer());
			ptrToNativeString = Marshal::StringToHGlobalAnsi(service);
			char* serviceString = static_cast<char*>(ptrToNativeString.ToPointer());
			ptrToNativeString = Marshal::StringToHGlobalAnsi(protocol);
			char* protocolString = static_cast<char*>(ptrToNativeString.ToPointer());

			// Map host name to remote IP address, allowing for dotted quad 
			long ihost;	   // 32-bit IP address number in Network Byte Order (big endian) 
			ihost = inet_addr(hostString);	// 1st try to convert from "dotted decimal notation" 
			if (ihost == INADDR_NONE) {	// if that doesn't work, send request to DNS 

				phe = gethostbyname(hostString);
				if (phe != NULL) { // If phe isn't NULL, DNS returned a matching IP address!
					memmove(&ihost, phe->h_addr, phe->h_length);
				}
				else {
					Console::WriteLine("Invalid host");
					return false;
				}
			}
			clientInfo.sin_addr.s_addr = ihost;	// Assign integer IP address to field in clientInfo 

												// Map service name (or number) to port number 
			pse = getservbyname(serviceString, protocolString);	// Search for matching service name
			if (pse != NULL) {						// If found, retrieve associated port number
				clientInfo.sin_port = (u_short)pse->s_port;
			}
			else {									// If not found, treat service as an integer
				short port = atoi(serviceString);
				if (port > 0) {
					clientInfo.sin_port = htons(port);
				}
				else {
					Console::WriteLine("Invalid service request");
					return false;
				}
			}

			// Connect the TCP socket
			int status = connect(s, (LPSOCKADDR)&clientInfo, sizeof(SOCKADDR));
			if (status == SOCKET_ERROR) {
				Console::WriteLine("Remote host/service not found - or connection refused.  Error Code = " + WSAGetLastError());
				return false;
			}
		}

		// If using UDP ...
		else if (protocol->Equals("udp")) {
			// Create a UDP socket (Nothing else to do!!)
			s = socket(AF_INET, SOCK_DGRAM, 0);
			if (s < 0 || s == INVALID_SOCKET) {
				Console::WriteLine("Cannot create socket.  Error Code = " + WSAGetLastError());
				return false;
			}
		}

		// This function doesn't support anything other than TCP or UDP
		else {
			return false;
		}

		return true;
	}

	Boolean Networking::strcmp_substr(char* lVal, char* rVal, int len) {
		for (int i = 0; i < len; i++) {
			if (lVal[i] != rVal[i] || lVal[i] == '\0' || rVal[i] == '\0') {
				return false;
			}
		}
		return true;
	}

	std::string Networking::c_substr(char* c_str, int begin, int end) {
		std::string retVal = c_str;
		if (end > begin)
			retVal = retVal.substr(begin, end - begin);
		else
			retVal = retVal.substr(begin);
		return retVal;
	}

	int Networking::strsize(char* in) {
		int i = 0;
		for (; in[i] != '\0'; i++) {}
		return i;
	}

	_bstr_t Networking::strToBstr(std::string in) {
		_bstr_t out(in.c_str());
		return out;
	}

	_bstr_t Networking::cStrToBstr(char* in) {
		_bstr_t out(in);
		return out;
	}
}