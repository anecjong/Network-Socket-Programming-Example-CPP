/**
 * @file 03-receiver.cpp
 * @brief UDP Broadcast Receiver implementation
 *
 * This file demonstrates reception of UDP broadcast messages.
 * It creates a UDP socket to receive messages sent to all hosts on a network.
 * It showcases:
 * - UDP socket creation
 * - Binding to INADDR_ANY to receive from any interface
 * - Setting socket options (SO_REUSEADDR) for shared port usage
 * - Receiving broadcast messages
 * - Handling data from multiple senders
 *
 * @note Uses SOCK_DGRAM socket type (UDP protocol)
 * @note Broadcasts are limited to the local network segment
 */

#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define BROADCAST_PORT 53772
#define SOCKET_OPTION_ENABLE_REUSEADDR 1
#define SOCKET_OPTION_ENABLE_REUSEPORT 1
#define INVALID_SOCKET -1
#define RECEIVE_ERROR -1

class BroadcastReceiver
{
public:
    BroadcastReceiver()
    {
        m_s32_socketFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_s32_socketFd == INVALID_SOCKET)
        {
            throw std::runtime_error("Failed to create socket");
        }

        // Allow multiple sockets to use the same PORT number
        int opt = SOCKET_OPTION_ENABLE_REUSEADDR;
        if (setsockopt(m_s32_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set socket options");
        }
        opt = SOCKET_OPTION_ENABLE_REUSEPORT;
        if (setsockopt(m_s32_socketFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set socket options");
        }

        // Bind to any address and the specified port
        m_clientAddress.sin_family = AF_INET;
        m_clientAddress.sin_addr.s_addr = INADDR_ANY;
        m_clientAddress.sin_port = htons(BROADCAST_PORT);

        if (bind(m_s32_socketFd, (struct sockaddr*)&m_clientAddress, sizeof(m_clientAddress)) == -1)
        {
            throw std::runtime_error("Failed to bind socket");
        }
    }

    ~BroadcastReceiver()
    {
        if (m_s32_socketFd != INVALID_SOCKET)
        {
            close(m_s32_socketFd);
        }
    }

    void receiveMessages()
    {
        char buffer[BUFFER_SIZE];
        struct sockaddr_in senderAddress;
        socklen_t senderLen = sizeof(senderAddress);

        std::cout << "Listening for broadcast messages on port " << BROADCAST_PORT << std::endl;
        while (true)
        {
            ssize_t bytesReceived =
                recvfrom(m_s32_socketFd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&senderAddress, &senderLen);

            if (bytesReceived == RECEIVE_ERROR)
            {
                std::cerr << "Failed to receive message" << std::endl;
                continue;
            }

            if (bytesReceived > BUFFER_SIZE - 1)
            {
                std::cerr << "Buffer overflow" << std::endl;
                continue;
            }

            buffer[bytesReceived] = '\0';
            std::cout << "Received from " << inet_ntoa(senderAddress.sin_addr) << ":" << ntohs(senderAddress.sin_port)
                      << " - " << buffer << std::endl;
        }
    }

private:
    int32_t m_s32_socketFd;
    struct sockaddr_in m_clientAddress;
};

int main()
{
    BroadcastReceiver receiver;
    while (true)
    {
        try
        {
            receiver.receiveMessages();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}