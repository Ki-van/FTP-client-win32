#pragma once
#include <winsock2.h>
class FTPClient
{
public:
	sockaddr_in saddr;
	void Connect(int port, char* adr);
	void Connect(int port, int adr);
	void SendMsg(char const *msg, int size);
	char* RecvMsg();
	void RecvMsg(char *buf, int size);
	void SaveFile(char filename[]);
	void CloseCon();
private:
	int sock;
	WSADATA wlib;
};