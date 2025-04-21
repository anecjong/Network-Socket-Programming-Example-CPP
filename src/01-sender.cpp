/**
 * @file 01-sender.cpp
 * @brief TCP Sender implementation
 *
 * This file demonstrates a simple TCP/IP sender implementation.
 * It creates a TCP socket, connects to a receiver, and sends/receives data
 * using stream-based communication. It showcases:
 * - TCP socket creation
 * - Connecting to a remote receiver
 * - Sending and receiving data over a TCP connection
 * - Socket cleanup
 *
 * @note Uses SOCK_STREAM socket type (TCP protocol)
 */

/**
 * TCP Sender
 *
 */
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define ANY_PORT 0

#define SOCKET_OPTION_ENABLE_REUSEADDR 1
#define SOCKET_OPTION_ENABLE_REUSEPORT 1
#define EXIT_COMMAND "exit"

int main(int argc, char** argv)
{
    int sockfd;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Failed to create socket\n";
        exit(EXIT_FAILURE);
    }

    int opt = SOCKET_OPTION_ENABLE_REUSEADDR;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set socket options\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    opt = SOCKET_OPTION_ENABLE_REUSEPORT;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set socket options\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    /*
    Set the socket option to reuse the address and port immediately after the program exits

    setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    level: SOL_SOCKET: socket layer
    optname: SO_REUSEADDR - Reuse the address immediately after the program exits
             SO_REUSEPORT - Reuse the port immediately after the program exits
    */

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(ANY_PORT);   // ANY PORT
    client_addr.sin_addr.s_addr = INADDR_ANY; // ANY IP
    if (bind(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1)
    {
        std::cerr << "Failed to bind socket\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to connect to server\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected to server\n";

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        std::cout << "Enter message to send to server: ";
        std::cin >> buffer;

        if (strcmp(buffer, EXIT_COMMAND) == 0)
        {
            break;
        }

        int bytes_sent = send(sockfd, buffer, strlen(buffer), 0);
        std::cout << "Bytes sent: " << bytes_sent << "\n";
        if (bytes_sent == -1)
        {
            std::cerr << "Failed to send message to server\n";
            break;
        }
        else if (bytes_sent == 0)
        {
            std::cout << "Server closed the connection\n";
            break;
        }
    }

    // Clean up
    close(sockfd);
    return 0;
}