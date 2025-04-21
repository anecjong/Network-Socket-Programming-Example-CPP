/**
 * @file 04-multicast.cpp
 * @brief Multicast Sender implementation
 *
 * This file demonstrates UDP multicast functionality.
 * It creates a UDP socket to send messages to a specific multicast group.
 * It showcases:
 * - UDP socket creation for multicast
 * - Setting the multicast TTL (Time To Live)
 * - Configuring multicast loopback
 * - Sending messages to a multicast address (238.238.238.238)
 * - One-to-many communication pattern
 *
 * @note Uses SOCK_DGRAM socket type (UDP protocol)
 * @note Multicast addresses range from 224.0.0.0 to 239.255.255.255
 * @note TTL determines how many network hops the packet can traverse
 */

#include <iostream>
#include <string>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 55555
#define MULTICAST_ADDRESS "238.238.238.238"
#define MULTICAST_PORT 55556
#define SOCKET_OPTION_ENABLE_REUSEADDR 1
#define SOCKET_OPTION_ENABLE_REUSEPORT 1

#define MULTICAST_TTL 32
#define MULTICAST_LOOPBACK_ENABLE 1
#define MULTICAST_LOOPBACK_DISABLE 0

/*
Multicast Addresses:
224.0.0.0 - 239.255.255.255
*/

class Multicast
{
public:
    Multicast()
    {
        m_s32_socketFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_s32_socketFd == -1)
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

        memset(&m_serverAddress, 0, sizeof(m_serverAddress));
        m_serverAddress.sin_family = AF_INET;
        m_serverAddress.sin_addr.s_addr = INADDR_ANY;
        m_serverAddress.sin_port = htons(SERVER_PORT);

        // 0: same host, 1: same subnet, over 1: can be transmitted to the other subnet. default is 1
        unsigned char ttl = MULTICAST_TTL; // Increased from 1 to 32 to allow more hops
        if (setsockopt(m_s32_socketFd, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, sizeof(ttl)) < 0)
        {
            throw std::runtime_error("Failed to set socket options: IP_MULTICAST_TTL" + std::string(strerror(errno)));
        }
        std::cout << "Set multicast TTL to " << (int)ttl << std::endl;

        unsigned char loopch = MULTICAST_LOOPBACK_ENABLE; // Changed to 1 to enable loopback for testing
        if (setsockopt(m_s32_socketFd, IPPROTO_IP, IP_MULTICAST_LOOP, (void*)&loopch, sizeof(loopch)) < 0)
        {
            throw std::runtime_error("Failed to set socket options: IP_MULTICAST_LOOP" + std::string(strerror(errno)));
        }
        std::cout << "Multicast loopback " << (loopch ? "enabled" : "disabled") << std::endl;

        if (bind(m_s32_socketFd, (struct sockaddr*)&m_serverAddress, sizeof(m_serverAddress)) == -1)
        {
            throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
        }

        memset(&m_multicastAddress, 0, sizeof(m_multicastAddress));
        m_multicastAddress.sin_family = AF_INET;
        m_multicastAddress.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
        m_multicastAddress.sin_port = htons(MULTICAST_PORT);
    }

    ~Multicast()
    {
        close(m_s32_socketFd);
    }

    void sendToMulticast(const std::string& message)
    {
        std::cout << "Sending message to " << MULTICAST_ADDRESS << ":" << MULTICAST_PORT << std::endl;
        if (sendto(m_s32_socketFd, message.c_str(), message.length(), 0, (struct sockaddr*)&m_multicastAddress,
                   sizeof(m_multicastAddress)) == -1)
        {
            std::cerr << "Failed to send message: " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to send message: " + std::string(strerror(errno)));
        }
    }

private:
    int32_t m_s32_socketFd;
    struct sockaddr_in m_serverAddress;
    struct sockaddr_in m_multicastAddress;
};

int main()
{
    Multicast multicast;

    while (true)
    {
        std::string message;
        std::cout << "Enter a message to send: ";
        std::getline(std::cin, message);
        multicast.sendToMulticast(message);
    }
    return 0;
}
