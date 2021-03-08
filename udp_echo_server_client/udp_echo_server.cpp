#include<iostream>
#include<sys/socket.h>
#include <sys/types.h>
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
    int socket_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_server < 0)
    {
        std::cerr << "[Error] Socket can not be created\n";
        return -2;
    }
    std::cout << "[INFO] Socket has been created!\n";

    //Address info to bind socket
    sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(std::atoi(argv[1]));
    inet_pton(AF_INET, "", &server_addr.sin_addr);

    char buf[INET_ADDRSTRLEN];
    //Bind socket
    if (bind(socket_server, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "[ERROR] Created socket can not be bind to ("
            << inet_ntop(AF_INET, &server_addr.sin_addr, buf, INET_ADDRSTRLEN)
            << ":" << ntohs(server_addr.sin_port) << ")\n";
        return -3;
    }
    std::cout << "[INFO] Sock is binded to ("
        << inet_ntop(AF_INET, &server_addr.sin_addr, buf, INET_ADDRSTRLEN)
        << ":" << ntohs(server_addr.sin_port) << ")\n";

    int client_addr_size = sizeof(client_addr);
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
    socklen_t len;
    //While receivng --- display and echo msg
    while (true)
    {
        bytes = recvfrom(socket_server, &msg_buf, 4096, 0, (sockaddr*)&client_addr, &len);
        //check how many bytes received
        //If there is no data, it means server is disconnected
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
           if (sendto(socket_server, &msg_buf, bytes, 0, (sockaddr*)&client_addr, sizeof(client_addr)) < 0)
            {
                std::cerr << "[ERROR] Message cannot be send, exiting...\n";
                break;
            }
        }
    }

    // Close socket
    close(socket_server);
    std::cout << "[INFO] Socket is closed.\n";

    return 0;
}
