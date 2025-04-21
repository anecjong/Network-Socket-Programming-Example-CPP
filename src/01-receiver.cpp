/**
 * @file 01-receiver.cpp
 * @brief TCP Receiver implementation
 *
 * This file demonstrates a simple TCP/IP receiver implementation.
 * It creates a TCP socket, binds to a port, listens for connections,
 * and handles sender communication. It showcases:
 * - TCP socket creation
 * - Socket binding and listening
 * - Accepting sender connections
 * - Sending and receiving data over a TCP connection
 * - Handling multiple senders
 * - Socket cleanup
 *
 * @note Uses SOCK_STREAM socket type (TCP protocol)
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
#define SOCKET_OPTION_ENABLE_REUSEADDR 1
#define SOCKET_OPTION_ENABLE_REUSEPORT 1
#define MAX_PENDING_CONNECTIONS 5

int main(int argc, char** argv)
{
    int sockfd;
    sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Failed to create socket\n";
        exit(EXIT_FAILURE);
    }
    /*
    sockfd: socket file descriptor

    socket(int domain, int type, int protocol)
    1st arg: Address Family
        AF_INET: IPv4
        AF_INET6: IPv6
    2nd arg: Socket Type
        SOCK_STREAM: TCP
        SOCK_DGRAM: UDP
    3rd arg: Protocol
        0: default protocol

    return socket descriptor if success
    return -1 if failed
    */

    int opt = SOCKET_OPTION_ENABLE_REUSEADDR;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set SO_REUSEADDR\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    opt = SOCKET_OPTION_ENABLE_REUSEPORT;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (void*)&opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set SO_REUSEPORT\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    /*
    Set the socket option to reuse the address and port immediately after the program exits

    setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    level: SOL_SOCKET: socket layer
    optname: SO_REUSEADDR - Reuse the address immediately after the program exits
             SO_REUSEPORT - Reuse the port immediately after the program exits
    optval: Pointer for the value of the option
    optlen: Length of the option value

    return 0 if success
    return -1 if failed
    */

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    /*
    sin_family: Address Family
    sin_port: Port number. 2 bytes(short). Value should be in network byte order(Big Endian).
    sin_addr.s_addr: IP address. 4 bytes
    INADDR_ANY: 0.0.0.0 - Allow all IP addresses
    */

    if (bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to bind socket\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    /*
    bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    return 0 if success
    return -1 if failed
    */

    if (listen(sockfd, MAX_PENDING_CONNECTIONS) == -1)
    {
        std::cerr << "Failed to listen\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    /*
    listen(int sockfd, int backlog)
    return 0 if success
    return -1 if failed
    */

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
    if (client_socket == -1)
    {
        std::cerr << "Failed to accept connection\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    /*
    accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)

    return client socket descriptor if success
    return -1 if failed
    */
    std::cout << "Connection accepted\n";

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == -1)
        {
            std::cerr << "Failed to receive message from client\n";
            close(client_socket);
            break;
        }
        else if (bytes_received == 0)
        {
            std::cout << "Client disconnected\n";
            close(client_socket);
            break;
        }
        std::cout << "Message from client: " << buffer << "\n";
    }

    // Clean up
    close(client_socket); // Close client socket
    close(sockfd);        // Close server socket
    return 0;
}