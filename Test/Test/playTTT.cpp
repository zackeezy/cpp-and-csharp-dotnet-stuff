// playTicTacToe.cpp
// This set of functions are used to actually play the game.
// Play starts with the function: playTicTacToe() which is defined below

#include "TicTacToe.h"
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <cctype>

int Chat(SOCKET s, std::string serverName, std::string remoteIP, std::string remotePort, std::string name, bool moveDef)
{
	bool myMove = moveDef;
	std::string temp;
	std::string message;
	do {
		if (myMove) {
			std::cout << "Input chat message: ";
			getline(std::cin, message);

			temp = message;

			for (char c : message) {
				c = tolower(c);
			}

			message = temp;
			int len1 = UDP_send(s, (char*)message.c_str(), message.length(), (char*)remoteIP.c_str(), (char*)remotePort.c_str());
		}
		else {
			std::cout << "Waiting for your opponent's move..." << std::endl << std::endl;
			int status = wait(s, WAIT_TIME, 0);
			if (status > 0) {
				char moveRecv[MAX_RECV_BUF];
				int len2 = UDP_recv(s, moveRecv, MAX_RECV_BUF, (char*)remoteIP.c_str(), (char*)remotePort.c_str());
				std::string check(moveRecv);
				for (char c : check) {
					c = tolower(c);
				}
				if (check == "quit") {
					return ABORT;
				}
				std::cout << name << ": " << moveRecv << std::endl;
				message = moveRecv;
			}
			else {
				return ABORT;
			}
		}
		myMove = !myMove;

		temp = message;

		for (char c : message) {
			c = tolower(c);
		}
	} while (message != "quit");

	return 0;
}