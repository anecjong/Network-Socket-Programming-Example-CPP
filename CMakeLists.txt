cmake_minimum_required(VERSION 3.10)
project(CXX_NETWORK_PROGRAMMING)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Simple TCP Receiver & Sender
add_executable(01-receiver src/01-receiver.cpp)
add_executable(01-sender src/01-sender.cpp)

# Raw Socket
add_executable(02-icmp src/02-icmp.cpp)

# Broadcast
add_executable(03-broadcast src/03-broadcast.cpp)
add_executable(03-receiver src/03-receiver.cpp)

# Multicast
add_executable(04-multicast src/04-multicast.cpp)
add_executable(04-receiver src/04-receiver.cpp)
