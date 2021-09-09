#include "FTPClient.h"

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include "FTPClient.h"
#include "Ws2tcpip.h"
#include "logger.h"
#pragma comment(lib, "Ws2_32.lib") //Winsock Library

void FTPClient::Connect(int port, char* adr) {
	WSAStartup(MAKEWORD(2, 2), &wlib);
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	inet_pton(AF_INET, adr, &saddr.sin_addr.s_addr);
	if (connect(sock, (SOCKADDR*)(&saddr), sizeof(saddr)) != 0) {
		cout << "No connection established." << endl;
		exit(1);
	}
	cout << "\nConnected to server on IP " << adr << " and port " << port << "." << endl;
}

void FTPClient::Connect(int port, int adr) {
	WSAStartup(MAKEWORD(2, 2), &wlib);
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = adr;
	
	if (connect(sock, (SOCKADDR*)(&saddr), sizeof(saddr)) != 0) {
		cout << "No connection established." << endl;
		exit(1);
	}
}


void FTPClient::SendMsg(char const *msg, int size) {
	if (send(sock, msg, size, 0) == -1) {
		cout << "Could not send message to server." << endl;
		exit(1);
	}
	cout << "Sent to server:\t\t";
	cout << msg;
}

char* FTPClient::RecvMsg() {
	char received[1024];
	RecvMsg(received, 1024);
	return received;
}

void FTPClient::RecvMsg(char* buf, int size) {
	int x;
	x = recv(sock, buf, size, 0); //recv() returns length of message
	buf[x] = '\0'; //0 indexing
	logA("%s", buf);
}

void FTPClient::SaveFile(char filename[]) {
	ofstream file;
	file.open(filename);
	char buffer[1];
	int bytes = 0;
	cout << "/*****FIRST 1 KB OF FILE**********************************************************************/" << endl;
	while (recv(sock, buffer, sizeof(buffer), 0)) {
		if (bytes < 1024)
			cout << buffer[0];
		file << buffer[0];
		bytes++;
	}
	cout << "\n/*********************************************************************************************/" << endl;
	file.close();
}


int FTPClient::RecvNextMLST(char* filefacts, u_int size)
{
	char buffer[2];
	buffer[1] = '\0';
	filefacts[0] = '\0';
	int bytes = 0;
	while (recv(sock, buffer, sizeof(char), 0)) {
		strcat_s(filefacts, size,  buffer);
		if (buffer[0] == '\n')
			break;
	}

	return strlen(filefacts);
}

void FTPClient::CloseCon() {
	closesocket(sock);
	WSACleanup();
	cout << "Closed connection.\n" << endl;
}