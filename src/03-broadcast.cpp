/**
 * @file 03-broadcast.cpp
 * @brief UDP Broadcast Sender implementation
 *
 * This file demonstrates UDP broadcast functionality.
 * It creates a UDP socket to send messages to all hosts on a network.
 * It showcases:
 * - UDP socket creation
 * - Enabling broadcast permissions with SO_BROADCAST
 * - Sending messages to broadcast address (255.255.255.255)
 * - One-to-all communication pattern
 *
 * @note Uses SOCK_DGRAM socket type (UDP protocol)
 * @note Broadcasts are limited to the local network segment
 */

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 53771
#define BROADCAST_ADDRESS "255.255.255.255"
#define BROADCAST_PORT 53772
#define BUFFER_SIZE 1024
#define SOCKET_OPTION_ENABLE_REUSEADDR 1
#define SOCKET_OPTION_ENABLE_REUSEPORT 1
#define BROADCAST_TIMEOUT_SECONDS 1
#define INVALID_SOCKET -1

class Broadcast
{
public:
    Broadcast()
    {
        m_s32_socketFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_s32_socketFd == INVALID_SOCKET)
        {
            std::cerr << "Failed to create socket" << std::endl;
            throw std::runtime_error("Failed to create socket");
        }

        int opt = SOCKET_OPTION_ENABLE_REUSEADDR;
        if (setsockopt(m_s32_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set socket options: REUSEADDR" + std::string(strerror(errno)));
        }
        opt = SOCKET_OPTION_ENABLE_REUSEPORT;
        if (setsockopt(m_s32_socketFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set socket options: REUSEPORT" + std::string(strerror(errno)));
        }

        if (setsockopt(m_s32_socketFd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set socket options: BROADCAST" + std::string(strerror(errno)));
        }

        m_serverAddress.sin_family = AF_INET;
        m_serverAddress.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        m_serverAddress.sin_port = htons(SERVER_PORT);

        if (bind(m_s32_socketFd, (struct sockaddr*)&m_serverAddress, sizeof(m_serverAddress)) == -1)
        {
            throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
        }

        m_broadcastAddress.sin_family = AF_INET;
        m_broadcastAddress.sin_addr.s_addr = inet_addr(BROADCAST_ADDRESS);
        m_broadcastAddress.sin_port = htons(BROADCAST_PORT);

        std::cout << "Server bound to: " << SERVER_ADDRESS << ":" << SERVER_PORT << std::endl;
        std::cout << "Broadcast address: " << BROADCAST_ADDRESS << ":" << BROADCAST_PORT << std::endl;
    }

    ~Broadcast()
    {
        if (m_s32_socketFd != INVALID_SOCKET)
        {
            close(m_s32_socketFd);
        }
    }

    void sendMessage(const std::string& message)
    {
        auto start = std::chrono::steady_clock::now();
        size_t message_length = message.length();
        int bytesSentTotal = 0;

        while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() <
               BROADCAST_TIMEOUT_SECONDS)
        {
            ssize_t bytesSent = sendto(m_s32_socketFd, message.c_str(), message.size(), 0,
                                       (struct sockaddr*)&m_broadcastAddress, sizeof(m_broadcastAddress));

            if (bytesSent == -1)
            {
                std::cerr << "Failed to send message" << std::endl;
                break;
            }

            bytesSentTotal += bytesSent;
            if (bytesSentTotal == message_length)
            {
                break;
            }
        }

        if (bytesSentTotal != message_length)
        {
            std::cerr << "Failed to send message" << std::endl;
        }
    }

private:
    int32_t m_s32_socketFd;
    struct sockaddr_in m_serverAddress;
    struct sockaddr_in m_broadcastAddress;
};

int main()
{
    Broadcast broadcast;
    while (true)
    {
        std::string message;
        std::cout << "Enter a message to send: ";
        std::getline(std::cin, message);
        broadcast.sendMessage(message);
    }
    return 0;
}
