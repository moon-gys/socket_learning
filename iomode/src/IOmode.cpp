#include"IOmode.h"

typedef int SOCKET;

IOmode::IOmode(int port, string ipaddr)
{
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(ipaddr.c_str());
}

IOmode::~IOmode()
{
}

void IOmode::blockmode()
{
    //Create a socket & get the file descriptor
    int sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server < 0)
    {
        std::cerr << "[Error] Socket can not be created\n";
        return -2;
    }
    std::cout << "[INFO] Socket has been created!\n";
   
    int flags = fcntl(sock_server, F_GETFL, 0);      
    fcntl(server, F_SETFL, flags | O_NONBLOCK);   

	//Bind socket
    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "[ERROR] Created socket can not be bind to ("
            << inet_ntop(AF_INET, &server_addr.sin_addr, buf, INET_ADDRSTRLEN)
            << ":" << ntohs(server_addr.sin_port) << ")\n";
        return -3;
    }
    std::cout << "[INFO] Sock is binded to ("
        << inet_ntop(AF_INET, &server_addr.sin_addr, buf, INET_ADDRSTRLEN)
        << ":" << ntohs(server_addr.sin_port) << ")\n";

	
    char buffer[1024];
    recvfrom(server, buffer, 1023, 0, (sockaddr*)&clientaddr, 0);

    // Start listening
    if (listen(server_fd, SOMAXCONN) < 0) {
        std::cerr << "[ERROR] Socket cannot be switched to listen mode!\n";
        return -4;
    }
    std::cout << "[INFO] Socket is listening now.\n";

    //Accept a call
    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    int sock_client;
    if ((sock_client = accept(server_fd, (sockaddr*)&client_addr, (socklen_t*)&client_addr_size)) < 0)
    {
        std::cerr << "[ERROR] Connection can not be accpted for a reason \n";
        return -5;
    }
    std::cout << "[INFO] A connection is accepted now.\n";

    // Close main listener socket
    close(server_fd);
    std::cout << "[INFO] Main listener socket is closed.\n";  
}

void IOmode::selectmode()
{
    vector<SOCKET> clientset;//客户机套接字集合
    map<SOCKET, sockaddr_in> s2addr;//套接字，地址
    fd_set readfd;//用于检查可读取数据的套接字集合
    int ret = 0;

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //unsigned long ul = 1;
    //ioctlsocket(server, FIONBIO, (unsigned long*)&ul);//设置server套接字为非阻塞，用于accept
    int flags = fcntl(server, F_GETFL, 0);        //获取文件的flags值。
    fcntl(server, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式；

    bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(server, 5);
    //不断接收连接与使用select检查各个客户机套接字是否有数据到来
    while (1)
    {
        sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int client = accept(server, (sockaddr*)&addr, &len);//接收连接
	if (client > 0 )
	{
	    //sort client_vec
	    sort(clientset.begin(),clientset.end());
	    clientset.push_back(client);
	    s2addr[client] = addr;
	    cout << client << " 已加入连接.." << "当前连接数: " << 
	    clientset.size() << endl;
	}
	FD_ZERO(&readfd);
	for (int i = 0; i < clientset.size(); i++)
	    FD_SET((int)(clientset[i]), &readfd);

	    //查看各个客户机是否发送数据过来。
	ret = 0;
	if (!clientset.empty())
	{
	    timeval tv = { 0,0 };
	    ret = select(clientset[clientset.size()-1] + 1, &readfd, NULL, NULL, &tv);
	}
	//处理接收数据
	if (ret > 0)
	{
	    vector<SOCKET> deleteclient;//要退出的套接字，待删除的套接字vector
	    for (int i = 0; i < clientset.size(); i++)
	        if (FD_ISSET((int)(clientset[i]), &readfd))
		{
		    char data[1024] = { 0 };
		    ret=recv(clientset[i], data, 1023, 0);
		    if(ret==0)
		    {

			deleteclient.push_back(clientset[i]);
		    }
		    if(ret>0)
		    {
		    cout<<clientset[i]<<" : "<<data<<endl;
		    }
		}

        //关闭要退出的套接字，在套接字集合中删除它
	if (!deleteclient.empty())
	{
	    for (int i = 0; i < deleteclient.size(); i++)
	    {
	        //closesocket(deleteclient[i]);
		cout<<deleteclient[i]<<" disconnected..."<<endl;
		vector<SOCKET>::iterator it = find(clientset.begin(), clientset.end(), deleteclient[i]);
		clientset.erase(it);
	    }
	}

        }
    }

}

void IOmode::sig_sigdrive(int sig)
{
    //signal come in, accept client connect
    sockaddr_in clientaddr;
    socklen_t len=sizeof(clientaddr);
    int client=accept(sigserver,(sockaddr*)&clientaddr,&len);

    if(client>0)
    {
        cout<<client<<" connected..."<<endl;
	sigclientset.push_back(client);
	sort(sigclientset.begin(),sigclientset.end());
    }

}

int IOmode::sigserver=-1;
vector<int> IOmode::sigclientset;

void IOmode::sigdrivemode()
{
    //socket...
    sigserver=socket(AF_INET,SOCK_STREAM,0);
    //开启服务器套接字信号，用于接收客户机的连接 
    struct sigaction SigAction;
    memset(&SigAction,0,sizeof(SigAction));
    SigAction.sa_flags=0;// set flags
    SigAction.sa_handler=sig_sigdrive;//信号发生时用于处理信号的函数
    sigaction(SIGIO,&SigAction,NULL);//register SIGIO signal
    //设置sigserver接收SIGIO信号信息,关联sigserver和信号
    fcntl(sigserver, F_SETOWN, getpid());
    //使用非阻塞IO和SIGIO信号发送
    int flags;
    flags = fcntl(sigserver, F_GETFL, 0);
    flags |= O_ASYNC | O_NONBLOCK;
    fcntl(sigserver, F_SETFL, flags);
    //bind，listen
    bind(sigserver,(sockaddr*)&serveraddr,sizeof(serveraddr));
    listen(sigserver,5);

    //handle IO,  use select...
    int ret;
    fd_set readfd;//用于检查可读取数据的套接字集合

    while(1)
    {
    	FD_ZERO(&readfd);
	for (int i = 0; i < sigclientset.size(); i++)
	    FD_SET((int)(sigclientset[i]), &readfd);
	ret=0;
	if (!sigclientset.empty())
	{
	    timeval tv = { 0,0 };
	    ret = select(sigclientset[sigclientset.size()-1] + 1, &readfd, NULL, NULL, &tv);
	}
	//处理接收数据
	if (ret > 0)
	{
	    vector<SOCKET> deleteclient;//要退出的套接字，待删除的套接字vector
	    for (int i = 0; i < sigclientset.size(); i++)
	        if (FD_ISSET((int)(sigclientset[i]), &readfd))
		{
		    char data[1024] = { 0 };
		    ret=recv(sigclientset[i], data, 1023, 0);
		    if(ret==0)
		    {
			deleteclient.push_back(sigclientset[i]);
		    }
		    if(ret>0)
		    {
		    cout<<sigclientset[i]<<" : "<<data<<endl;
		    }
		}

        //关闭要退出的套接字，在套接字集合中删除它
	if (!deleteclient.empty())
	{
	    for (int i = 0; i < deleteclient.size(); i++)
	    {
	        //closesocket(deleteclient[i]);
		cout<<deleteclient[i]<<" disconnected..."<<endl;
		vector<SOCKET>::iterator it = find(sigclientset.begin(), sigclientset.end(), deleteclient[i]);
		sigclientset.erase(it);
	    }
	}

	}
    }
}

void IOmode::pollmode()
{
    char recvdata[1024]={0};
    int ret = 0,socketnum=0;//return result; socket counts
    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(server, 5);
    
    struct pollfd socketset[1024];//socket set,socketset[0]:server,socketset[else]:clients
    
    socketset[0].fd=server;
    socketset[0].events=POLLIN;
    socketnum=1;
    for(int i=1;i<1024;i++)
	socketset[i].fd=-1;
    while(1)
    {
	ret=poll(socketset,socketnum,-1);
	if(ret<0)
	{
	    cout<<"poll error"<<endl;return;
	}    
	//accept connect
	if(socketset[0].revents & POLLIN)
	{
	    socketset[0].revents=0;
	    sockaddr_in clientaddr;
	    socklen_t len=sizeof(clientaddr);
	    int client=accept(server,(sockaddr*)&clientaddr,&len);
	    if(client)
	    {
		//add it in new pollfd;
		for(ret=1;ret<1024;ret++)
		    if(socketset[ret].fd==-1)
			break;
		if(ret>=1024)
		    {cout<<"full connected.."<<endl;continue;}
		socketset[ret].fd=client;socketnum++;
		socketset[ret].events=POLLIN;
		cout<<client<<" connected..."<<endl<<
		"rest clients: "<<socketnum-1<<endl;
		//cout<<socketnum<<endl;
	    }
	}
	//recv data
	for(int i=1;i<socketnum;i++)
	{
	    if((socketset[i].revents & POLLIN) && socketset[i].fd>0)
	    {
		socketset[i].revents=0;
		ret=recv(socketset[i].fd,recvdata,1023,0);
		if(ret==0)
		{
		    //close socket....
		    cout<<socketset[i].fd<<" disconnected..."<<
		    "rest clients: "<<socketnum-2<<endl;
		    socketset[i].fd=-1;socketnum--;
		}
		if(ret>0)
		    cout<<socketset[i].fd<<" : "<<recvdata<<endl;
	    }
	}
    }


}

void IOmode::epollmode()
{

    char recvdata[1024]={0};
    int ret = 0;//return result; 
	//socket，bind，listen
    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(server, (sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(server, 5);
    //创建一个epollfd，所有套接字的I/O通知都靠这个epollfd
    int epollfd=epoll_create(1024);
    struct epoll_event ev;
    //注册epoll_event并把server关联到之前的epollfd上等待客户机连接的通知
	ev.events=EPOLLIN;
    ev.data.fd=server;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,server,&ev);
    //用于接收epoll通知的结果，保存套接字的集合
    struct epoll_event evs[1024];
    
    while(1)
    {
		//epoll_wait返回发生了I/O操作的套接字的数目，
		//套接字描述符与网络事件信息保存在套接字描述符集合evs中
        ret=epoll_wait(epollfd,evs,1024,-1);
    	if(ret<0)
	{
	    cout<<"poll error"<<endl;return;
	}    
	//对发生了I/O的各个 套接字进行相应的处理
	for(int i=0;i<ret;i++)
	{
		//处理服务器套接字，处理accept
	    if(evs[i].data.fd==server && evs[i].events & EPOLLIN)
	    {
		sockaddr_in clientaddr;
	    	socklen_t len=sizeof(clientaddr);
	    	int client=accept(server,(sockaddr*)&clientaddr,&len);
	    	if(client)
	    	{
		    //非常重要，需要把新连进来的客户机关联到之前的epollfd上 ，等待读数据的通知
		    ev.data.fd=client;
		    ev.events=EPOLLIN;
		    epoll_ctl(epollfd,EPOLL_CTL_ADD,client,&ev);
		    cout<<client<<" connected..."<<endl;
		}
	    }
		//处理客户机套接字，处理recv
	    if(evs[i].data.fd!=server && evs[i].events & EPOLLIN)
	    {
		ret=recv(evs[i].data.fd,recvdata,1023,0);
		if(ret==0)
		{
		    //关闭套接字，取消该  客户机对epollfd的关联。
		    cout<<evs[i].data.fd<<" disconnected..."<<endl;
		    close(evs[i].data.fd);
		    epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[i].data.fd,NULL);
		}
		if(ret)
		    cout<<evs[i].data.fd<<" : "<<recvdata<<endl;
	    }
	}
    }
}









