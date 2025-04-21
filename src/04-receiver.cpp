/**
 * @file 04-receiver.cpp
 * @brief Multicast Receiver implementation
 *
 * This file demonstrates reception of UDP multicast messages.
 * It creates a UDP socket to receive messages sent to a specific multicast group.
 * It showcases:
 * - UDP socket creation for multicast
 * - Setting socket options (SO_REUSEADDR, SO_REUSEPORT) for shared port usage
 * - Joining a multicast group using IP_ADD_MEMBERSHIP
 * - Receiving messages from the multicast group
 * - Leaving the multicast group with IP_DROP_MEMBERSHIP
 *
 * @note Uses SOCK_DGRAM socket type (UDP protocol)
 * @note Must bind to the same port that the sender uses for multicast
 * @note Multiple receivers can join the same multicast group
 */

#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MULTICAST_ADDRESS "238.238.238.238"
#define MULTICAST_PORT 55556
#define SOCKET_OPTION_ENABLE_REUSEADDR 1
#define SOCKET_OPTION_ENABLE_REUSEPORT 1
#define BUFFER_SIZE 1024

class MulticastReceiver
{
public:
    MulticastReceiver()
    {
        m_sockFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_sockFd == -1)
        {
            throw std::runtime_error("Failed to create socket");
        }

        int opt = SOCKET_OPTION_ENABLE_REUSEADDR;
        if (setsockopt(m_sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set SO_REUSEADDR");
        }
        opt = SOCKET_OPTION_ENABLE_REUSEPORT;
        if (setsockopt(m_sockFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        {
            throw std::runtime_error("Failed to set SO_REUSEPORT");
        }

        m_receiverAddress.sin_family = AF_INET;
        m_receiverAddress.sin_addr.s_addr = INADDR_ANY;
        m_receiverAddress.sin_port = htons(MULTICAST_PORT); // MUST BE SAME AS THE PORT IN MULTICAST SENDER

        if (bind(m_sockFd, (struct sockaddr*)&m_receiverAddress, sizeof(m_receiverAddress)) == -1)
        {
            throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
        }

        // Set up multicast group membership
        m_mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDRESS);
        m_mreq.imr_interface.s_addr = INADDR_ANY;

        if (setsockopt(m_sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m_mreq, sizeof(m_mreq)) == -1)
        {
            std::cerr << "Failed to join multicast group: " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to join multicast group: " + std::string(strerror(errno)));
        }
        std::cout << "Successfully joined multicast group " << MULTICAST_ADDRESS << std::endl;
        std::cout << "Listening for multicast messages on " << MULTICAST_ADDRESS << ":" << MULTICAST_PORT << std::endl;
    }

    ~MulticastReceiver()
    {
        if (setsockopt(m_sockFd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&m_mreq, sizeof(m_mreq)) < 0)
        {
            std::cerr << "Failed to drop multicast group: " << std::string(strerror(errno)) << std::endl;
        }
        else
        {
            std::cout << "Left multicast group " << MULTICAST_ADDRESS << std::endl;
        }
        close(m_sockFd);
    }

    void receiveMessages()
    {
        char buffer[BUFFER_SIZE];
        struct sockaddr_in senderAddress;
        socklen_t senderLen = sizeof(senderAddress);

        std::cout << "Waiting for multicast messages..." << std::endl;
        ssize_t bytesRead = recvfrom(m_sockFd, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddress, &senderLen);

        if (bytesRead == -1)
        {
            std::cerr << "Failed to receive message: " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to receive message");
        }

        std::cout << "Received message from " << inet_ntoa(senderAddress.sin_addr) << ":"
                  << ntohs(senderAddress.sin_port) << std::endl;
        std::cout << "Message: " << std::string(buffer, bytesRead) << std::endl;
    }

private:
    int m_sockFd;
    struct sockaddr_in m_receiverAddress;
    struct ip_mreq m_mreq;
};

int main(int argc, char* argv[])
{
    MulticastReceiver receiver;

    while (true)
    {
        receiver.receiveMessages();
    }

    return 0;
}
