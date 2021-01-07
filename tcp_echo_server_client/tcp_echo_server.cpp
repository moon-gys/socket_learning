#include<iostream>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>

//Check if a c-style string is an integer
bool is_int(char* c)
{
    while (*c != '\0')
    {
        if (*c < 0 || *c > 9)
            return false;
        c++;
    }

    return true;
}

int main(int argc, char** argv)
{
    if (argc != 2 || is_int(argv[1]))
    {
        std::cerr << " [Error] Port is not provided via command line paramters!\n";

        return -1;
    }

    //Create a socket & get the file descriptor
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "[Error] Socket can not be created\n";
        return -2;
    }
    std::cout << "[INFO] Socket has been created!\n";

    //Address info to bind socket
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::atoi(argv[1]));
    inet_pton(AF_INET, "10.93.13.217", &server_addr.sin_addr);

    char buf[INET_ADDRSTRLEN];
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

    // Get name info
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    if (getnameinfo(
        (sockaddr*)&client_addr, client_addr_size,
        host, NI_MAXHOST,
        svc, NI_MAXSERV, 0) != 0) {
        std::cout << "[INFO] Client: (" << inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN)
            << ":" << ntohs(client_addr.sin_port) << ")\n";
    }
    else {
        std::cout << "[INFO] Client: (host: " << host << ", service: " << svc << ")\n";
    }

    char msg_buf[4096];
    int bytes;
    //While receivng --- display and echo msg
    while (true)
    {
        bytes = recv(sock_client, &msg_buf, 4096, 0);

        //check how many bytes received, If there is no data, it means server is disconnected
        if (bytes == 0)
        {
            std::cerr << "[INFO] Client is disconnected\n";
            break;
        }
        else if (bytes < 0)
        {
            std::cerr << "[Error] Something went wrong when receiving data\n";
            break;
        }
        else
        {
            std::cout << "client " << std::string(msg_buf, 0, bytes) << "\n";
            //Resend the same message
            if (send(sock_client, &msg_buf, bytes, 0) < 0)
            {
                std::cerr << "[ERROR] Message cannot be send, exiting...\n";
                break;
            }
        }

    }

    //close client socket
    close(sock_client);
    std::cout << "[INFO] Client socket is closed\n";

    return 0;
}
