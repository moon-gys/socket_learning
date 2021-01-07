#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>

#include <sys/types.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <windows.h> 

#pragma comment(lib, "WS2_32")

// Note: Check for SIGPIPE and other socket related signals

// Checks If a c-style string is an integer
bool is_int(char* c) {
    while (*c != '\0')
    {
        if (*c < '0' || *c > '9')
            return false;
        c++;
    }
    return true;
}

int main(int argc, char** argv) {
    // Check arguments
    if (argc < 3 || !is_int(argv[2])) {
        std::cerr << "[ERROR] Parameters are not valid!\n";
        return -1;
    }

    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    // Address and host info 
    sockaddr_in server_addr;
    hostent* server;

    // Create a socket & get the file descriptor
    SOCKET sock_client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // Check If the socket is created
    if (sock_client < 0) {
        std::cerr << "[ERROR] Socket cannot be created!\n";
        return -2;
    }
    std::cout << "[INFO] Socket has been created.\n";

    int port{ std::atoi(argv[2]) };
    // Get host information by name
    // gethostbyname is not thread-safe, checkout getaddrinfo
    server = gethostbyname(argv[1]);
    if (!server) {
        std::cerr << "[ERROR] No such host!\n";
        return -3;
    }
    std::cout << "[INFO] Hostname is found.\n";

    // Fill address fields before try to connect
    //std::memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.S_un.S_addr = inet_addr("10.93.13.217");

    // Check if there is an address of the host
    if (server->h_addr_list[0])
        std::memcpy((char*)server->h_addr_list[0], (char*)&server_addr.sin_addr, server->h_length);
    else {
        std::cerr << "[ERROR] There is no a valid address for that hostname!\n";
        return -5;
    }
  
    char buf[4096]{ "test!" };
    std::string temp;
    // While user input is not empty or "quit"
    // Send data to the server
    // Wait for a message/response
    // Print the response
    do {
        // Clear buffer, get user input
        std::memset(buf, 0, 4096);
        std::cout << "> ";
        std::getline(std::cin, temp, '\n');
        std::strcpy(buf, temp.c_str());

        // Check the input
        if (!strlen(buf)) continue;
        else if (!strcmp(buf, "quit")) break;

        // Send the data that buffer contains
        if (sendto(sock_client, buf, (size_t)strlen(buf), 0, (sockaddr*)&server_addr, sizeof(server_addr)))
        {
            // Wait for a message
            int server_addr_size = sizeof(server_addr);

            // Call the recvfrom function to receive datagrams on the bound socket.
            wprintf(L"Receiving datagrams...\n");
            int bytes_recv = recvfrom(sock_client,
                buf, 4096, 0, (SOCKADDR*)&server_addr, &server_addr_size);
            if (bytes_recv == SOCKET_ERROR) {
                wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
            }
           // int bytes_recv = recvfrom(sock_client, buf, 4096, 0, (sockaddr*)&server_addr, &len);

            //check how many bytes received
        //If there is no data, it means server is disconnected
            if (bytes_recv == 0)
            {
                std::cerr << "[INFO] Client is disconnected\n";
                break;
            }

            //If something gone wrong
            else if (bytes_recv < 0)
            {
                std::cerr << "[Error] Something went wrong when receiving data\n";
                break;
            }

            //If there is some bytes
            else
                std::cout << "server: " << std::string(buf, 0, bytes_recv) << "\n";

        }
     
    } while (true);

    // Close socket
    closesocket(sock_client);
    std::cout << "[INFO] Socket is closed.\n";

    WSACleanup();

    return 0;
}
