#ifndef _IOMODE_H_
#define _IOMODE_H_

#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include<unistd.h>
#include<fcntl.h>
#include<signal.h>
#include<poll.h>
#include<sys/epoll.h>

#include<string.h>
#include<vector>
#include<algorithm>
#include<map>
#include<iostream>


using namespace std;


class IOmode
{
private:
	sockaddr_in server_addr;
	static void sig_sigdrive(int);
	static void sig_async(int);
	//use sigxxx in sigdrivemode	
	static int sigserver;
	static vector<int> sigclientset;

public:
	IOmode(int port, string ipaddr);

	~IOmode();

	void blockmode();

	void selectmode();

	void sigdrivemode();

	void pollmode();

	void epollmode();
};


#endif