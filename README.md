# Socket Programming Test Codes

A collection of C++ examples demonstrating various network socket programming techniques. These examples cover fundamental networking concepts and provide a practical introduction to socket programming in C++.

## Overview

This repository contains implementation examples for:

- TCP sender/receiver communication
- ICMP (ping) implementation
- UDP broadcast messaging
- Multicast group communication

## Examples

### TCP Communication (01-*.cpp)

- **01-receiver.cpp**: TCP receiver implementation that accepts sender connections and handles communication
- **01-sender.cpp**: TCP sender implementation that connects to a TCP receiver and exchanges messages

### ICMP Implementation (02-*.cpp)

- **02-icmp.cpp**: Raw socket implementation for ICMP echo request/reply (similar to ping utility)

### Broadcast Communication (03-*.cpp)

- **03-broadcast.cpp**: UDP broadcast sender that transmits messages to all hosts on a local network
- **03-receiver.cpp**: Broadcast receiver that listens for messages sent to the broadcast address

### Multicast Communication (04-*.cpp)

- **04-multicast.cpp**: Multicast sender that transmits messages to a specific multicast group
- **04-receiver.cpp**: Multicast receiver that joins a multicast group and receives messages

## Compilation

This project uses CMake for building. Follow these steps to compile all examples:

```bash
mkdir build && cd build
cmake ..
make
```

This will build all the executables in the build directory:
- 01-receiver
- 01-sender
- 02-icmp
- 03-broadcast
- 03-receiver
- 04-multicast
- 04-receiver

## Usage Examples

### TCP Sender/Receiver

1. Start the receiver in one terminal:
```bash
./01-receiver
```

2. Connect with the sender in another terminal:
```bash
./01-sender
```

3. Type messages in the sender terminal to send to the receiver.

### ICMP (Ping)

Requires root/sudo privileges to create raw sockets:

```bash
sudo ./02-icmp
```

### UDP Broadcast

1. Start the receiver in one terminal:
```bash
./03-receiver
```

2. Send broadcast messages in another terminal:
```bash
./03-broadcast
```

### Multicast

1. Start the multicast receiver:
```bash
./04-receiver
```

2. Send multicast messages:
```bash
./04-multicast
```

## Key Networking Concepts

### Socket Types
- **SOCK_STREAM**: Connection-oriented, reliable (TCP)
- **SOCK_DGRAM**: Connectionless, unreliable (UDP)
- **Raw Sockets**: Low-level network protocol access (ICMP)

### Communication Patterns
- **Unicast**: One-to-one (TCP)
- **Broadcast**: One-to-all in a network (UDP broadcast)
- **Multicast**: One-to-many specific subscribed receivers (UDP multicast)

## Important Notes

1. **Multicast Configuration**:
   - Set `IP_MULTICAST_LOOP` to 1 when running sender and receiver on the same machine

2. **ICMP Implementation**:
   - Requires root/administrator privileges

## License

This code is provided for educational purposes only. No warranties or guarantees of any kind are provided. Use at your own risk.

### Usage Rights
- MIT License
- You are free to use, modify, and distribute this code for personal, educational, or commercial purposes.
- Attribution is appreciated but not required.
- If you find this code useful in your projects or publications, you may reference it as:
  ```
  Socket Programming Examples from https://github.com/anecjong/Network-Socket-Programming-Example-CPP
  ```