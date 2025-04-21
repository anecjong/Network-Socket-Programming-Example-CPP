/**
 * @file 02-icmp.cpp
 * @brief ICMP Protocol implementation
 *
 * This file demonstrates Internet Control Message Protocol (ICMP) implementation.
 * It creates raw sockets for sending and receiving ICMP packets, similar to
 * the 'ping' utility. It showcases:
 * - Raw socket creation for ICMP
 * - ICMP packet construction
 * - Calculating checksums for network packets
 * - Sending ICMP echo requests
 * - Receiving and processing ICMP echo replies
 * - Network diagnostics and round-trip time measurement
 *
 * @note Uses raw sockets which typically require root/administrator privileges
 */

#include <arpa/inet.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#define GOOGLE_DNS "8.8.8.8"
#define PACKET_SIZE 56
#define MAX_WAIT_TIME 1000 // milliseconds

#define BUFFER_SIZE 1024
#define ADDRESS "127.0.0.1"
#define PORT 8080

// Function to calculate ICMP checksum
unsigned short calculate_checksum(void* b, int len)
{
    unsigned short* buf = (unsigned short*)b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
    {
        sum += *buf++;
    }
    if (len == 1)
    {
        sum += *(unsigned char*)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char** argv)
{
    int sockfd;
    struct sockaddr_in source_addr;
    struct sockaddr_in target_addr;
    char send_buffer[PACKET_SIZE + sizeof(struct icmphdr)];
    char recv_buffer[BUFFER_SIZE];
    int send_count = 10;
    int sequence = 0;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1)
    {
        std::cerr << "Failed to create socket\n";
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set socket options\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set socket options\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    source_addr.sin_family = AF_INET;
    source_addr.sin_port = htons(PORT);
    source_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&source_addr, sizeof(source_addr)) == -1)
    {
        std::cerr << "Failed to bind socket\n";
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        std::cerr << "Failed to set SO_RCVTIMEO\n";
        exit(EXIT_FAILURE);
    }

    // Port is not needed for ICMP - Only IP address is needed
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(GOOGLE_DNS);

    // Initialize ICMP header
    struct icmphdr* icmp_header = (struct icmphdr*)send_buffer;
    icmp_header->type = ICMP_ECHO;
    icmp_header->code = 0;
    icmp_header->un.echo.id = getpid() & 0xFFFF;

    // Fill payload with some data
    // Detecting packet corruption
    // Verifying the entire packet was received correctly
    // Making the packet size consistent with standard ping (56 bytes payload + 8 bytes ICMP header = 64 bytes total)
    for (int i = sizeof(struct icmphdr); i < PACKET_SIZE + sizeof(struct icmphdr); i++)
    {
        send_buffer[i] = i % 256;
    }

    while (send_count > 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Update sequence number and calculate checksum
        icmp_header->un.echo.sequence = sequence;
        icmp_header->checksum = 0;
        icmp_header->checksum = calculate_checksum(icmp_header, PACKET_SIZE + sizeof(struct icmphdr));

        auto start = std::chrono::high_resolution_clock::now();

        if (sendto(sockfd, send_buffer, PACKET_SIZE + sizeof(struct icmphdr), 0, (struct sockaddr*)&target_addr,
                   sizeof(target_addr)) <= 0)
        {
            std::cerr << "Failed to send ICMP packet\n";
            continue;
        }

        bool received = false;
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start)
                   .count() < MAX_WAIT_TIME)
        {
            int recv_len = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0, NULL, NULL);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

            if (recv_len == -1)
            {
                received = false;
                break;
            }
            else
            {
                struct iphdr* ip_header = (struct iphdr*)recv_buffer;
                struct icmphdr* recv_icmp = (struct icmphdr*)(recv_buffer + (ip_header->ihl * 4));

                if (recv_icmp->type == ICMP_ECHOREPLY && recv_icmp->un.echo.id == (getpid() & 0xFFFF) &&
                    recv_icmp->un.echo.sequence == sequence)
                {
                    unsigned short original_checksum = recv_icmp->checksum;
                    recv_icmp->checksum = 0;
                    unsigned short calculated_checksum = calculate_checksum(recv_icmp, recv_len - (ip_header->ihl * 4));
                    recv_icmp->checksum = original_checksum;

                    if (original_checksum == calculated_checksum)
                    {
                        std::cout << "64 bytes from " << inet_ntoa(target_addr.sin_addr)
                                  << ": icmp_seq=" << recv_icmp->un.echo.sequence << " ttl=" << (int)ip_header->ttl
                                  << " time=" << std::fixed << std::setprecision(2) << duration.count() / 1000000.0
                                  << " ms\n";
                        received = true;
                        break;
                    }
                }
            }
        }

        if (!received)
        {
            std::cerr << "Request timed out\n";
        }
    }

    close(sockfd);
    return 0;
}