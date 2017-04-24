// Test.h

#pragma once
#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <comdef.h>

#pragma comment (lib, "iphlpapi.lib")
#pragma comment (lib, "ws2_32.lib")
#define MAX_IP_ADDRESS_LEN 16
#define MAX_PORT_LEN 6

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Test {

	public ref class Networking
	{
	public:
		Boolean WSAInit();
		Int32 WSACleanupWrapper();
		Int32 wait(Int32 seconds, Int32 msec);
		Int32 UDP_send(String ^buf, Int32 numBytesToSend, String ^host, String ^service);
		Int32 UDP_recv(String ^buf, Int32 maxsize, String ^host, String ^port);
		Boolean initPassiveSock(String ^service, String ^protocol);
		String ^getBroadcastAddress();
		Boolean initSock(String ^host, String ^service, String ^protocol);
		static const String ^TicTacToe_QUERY = "Who?";
		static const String ^TicTacToe_NAME = "Name=";
		static const String ^TicTacToe_CHALLENGE = "Player=";
		static const Int32 v4AddressSize = 16;
		static const Int32 MAX_INTERFACES = 20;
		static const Int32 WAIT_TIME = 60;
		static const Int32 MAX_SEND_BUF = 2048;
		static const Int32 MAX_RECV_BUF = 2048;
		static const Int32 MAX_SERVERS = 100;
		static const Int32 ABORT = -1;
		static const String ^TicTacToe_UDPPORT = "29333";

	private:
		bool strcmp_substr(char* lVal, char* rVal, int len);
		std::string c_substr(char* c_str, int begin, int end);
		int strsize(char* in);
		_bstr_t strToBstr(std::string in);
		_bstr_t cStrToBstr(char* in);
		static SOCKET s;
	};

	//public ref class NimGame {
	//	Array<int> rows;
	//	//This number is specified by the host.
	//	int row_count = 4;

	//	/*
	//	This function uses the vector and the global(now class) value rows
	//	Iterates thru the vector and adds the number of rocks in total
	//	returns the sum of the rocks
	//	(to check to see if the game is over, i.e. 0 rocks are left)
	//	*/
	//	int sum() {
	//		int value = 0;
	//		for (unsigned int i = 0; i<rows.size(); i++) {
	//			value += rows[i];
	//		}
	//		return value;
	//	}

	//	/*
	//	This function will draw to console the rows and how many rocks are per row
	//	this uses the global variables so it does not need to take parameters as is
	//	*/
	//	void draw_game() {
	//		for (int i = 0; i < rows.size(); i++) {
	//			cout << i + 1 << ": ";
	//			for (int j = 0; j < rows[i]; j++) {
	//				cout << " * ";
	//			}
	//			cout << endl;
	//		}
	//	}

	//	/*
	//	This function will display a cout statement and return the value given by user
	//	*/
	//	void input(int &command, const char* command_name) {
	//		cout << "Press " << command_name << ". Use 0 to exit." << endl;
	//		cin >> command;
	//	}

	//	/*This function will take:
	//	1- Row number: Describes the row where we want to extract rocks from
	//	2- number: The amount of rocks we want to take from that rock
	//	Return: Bool describing is the subtraction was sucessful
	//	*/
	//	bool remove_element(int row, int number) {

	//		if (row < rows.size()) {
	//			if (number <= rows[row]) {
	//				rows[row] -= number;
	//				return true;
	//			}
	//		}
	//		cout << "Please check your input and try again." << endl;
	//		return false;
	//	}


	//	/*
	//	This is a little AI model I found online, so now you can play the nim game with yourself
	//	*/
	//	int xOr() {
	//		int value = 0;
	//		for (unsigned int i = 0; i<rows.size(); i++) {
	//			value ^= rows[i];
	//		}
	//		return value;
	//	}

	//	void add_element(int row, int number) {
	//		rows[row] += number;
	//	}


	//	void computer() {
	//		cout << "Computer turn..." << endl;
	//		int sum = xOr();
	//		for (int i = 0; i < rows.size(); i++) {
	//			if (rows[i] > 0) {
	//				for (int j = 1; j <= rows[i]; j++) {
	//					remove_element(i, j);
	//					int sum = xOr();
	//					if (sum != 0) {
	//						add_element(i, j);
	//					}
	//					else {
	//						cout << "Removed " << j << " from row " << i + 1 << endl;
	//						return;
	//					}
	//				}
	//			}
	//		}
	//		int i = 0;
	//		while (!remove_element(i, 1))
	//			i++;
	//	}
	//	//END of AI

	//public:
	//	int playgame() {

	//		//Populating the vector:
	//		//Sample String "4 10 02 10 14" 4 rows, 1-) 10 || 2-) 02 || 3-) 10 || 4-) 14
	//		//NOTE: The requirements say that string must be 19 length the most, when receiveing the data please check for that to be true
	//		char received_string[] = "410021014";

	//		row_count = received_string[0] - '0';

	//		rsize_t length = sizeof(received_string) - 2;
	//		//Max number of piles is two
	//		int rows_received[9];

	//		//Getting the size of all the rows
	//		int index = 0;
	//		for (int i = 0; i < length / 2; i++) {
	//			rows_received[i] = ((received_string[index + 1] - '0') * 10) + (received_string[index + 2] - '0');
	//			index += 2;
	//		}
	//		//Populating the vector
	//		for (int i = row_count - 1; i >= 0; i--) {
	//			rows.push_back(rows_received[i]);
	//		}

	//		cout << "Game Prototype" << endl;

	//		int command = -1;
	//		bool validated = false;
	//		while (command != 0 || !validated) {

	//			draw_game();
	//			input(command, "row number");
	//			if (command != 0) {
	//				int row = command;
	//				input(command, "how much you want to remove");
	//				if (command != 0) {
	//					validated = remove_element(row - 1, command);
	//					//Check to see is game is over
	//					if (sum() == 0) {
	//						cout << "You win." << endl;
	//						command = 0;
	//						break;
	//					}
	//					else {
	//						//Validate will be true if a valid turn just passed computer will make its move
	//						if (validated) computer();
	//						//Check to see if game is over
	//						if (sum() == 0) {
	//							cout << "You loose." << endl;
	//							command = 0;
	//						}
	//					}
	//				}
	//			}
	//			else {
	//				return 0;
	//			}
	//		}
	//		return 0;
	//	}
	//};

	struct ServerStruct {
		std::string name;
		std::string host;
		std::string port;
	};
}
