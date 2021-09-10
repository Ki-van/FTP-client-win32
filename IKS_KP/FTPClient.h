#pragma once
#include <winsock2.h>
class FTPClient
{
public:
	sockaddr_in saddr;
	void  Connect(int port, char* adr);
	void  Connect(int port, int adr);
	void  Connect(wchar_t* port, wchar_t *adr);
	void  SendMsg(char const *msg, int size);
	char* RecvMsg();
	void  RecvMsg(char *buf, int size);
	void  SaveFile(wchar_t* filename);
	void  CloseCon();
	int  RecvNextMLST(char *filefact, u_int size);
private:
	int sock;
	WSADATA wlib;
};