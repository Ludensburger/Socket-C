# Many-to-One Socket Programming for BlackJack Game

## Overview

This program demonstrates a Many-to-One connection setup for socket programming. One main server (also referred to as "Player 1" or "Primary Player") will host the session. Multiple clients (other computers) will connect to the server using their IP addresses.

## Smart Algorithm

The program incorporates a smart algorithm for the dealer's decision-making process in BlackJack. The dealer uses a probabilistic model to decide whether to hit or stand based on its current score and the visible scores of the players. This adds an element of strategy and intelligence to the game, making it more challenging and engaging for the players.

## Requirements

1. The **host machine** (server) must set its IPV4 address as the `SERVER_IP`.

   - Example: If I am the host, I will set my IPV4 address as `192.168.0.1`.

2. All **client machines** must have IPV4 addresses in the same local network as the `SERVER_IP`.
   - Ensure your IPV4 address matches the network of the `SERVER_IP`.
   - Specifically, this means your address must be within the usable host range, starting from the first usable address after the network identifier up to the address before the broadcast address.
   - Compile the `client.c` file: `gcc client.c -o client`.
   - Run the client executable: `./client` or `client.exe`.

## Instructions

### For the Host (Server)

1. Set your machine's IPV4 address as `SERVER_IP` in the source code.
2. Compile the `server.c` file: `gcc server.c -o server`.
3. Run the server executable: `./server` or `server.exe`.

### For the Clients

1. Ensure your IPV4 address matches the network of the `SERVER_IP`.
   - Specifically, this means your address must be within the usable host range, starting from the first usable address after the network identifier up to the address before the broadcast address.
2. Compile the `client.c` file: `gcc client.c -o client`.
3. Run the client executable: `./client` or `client.exe`.

## Network Troubleshooting

- If a client cannot connect:
  1. Confirm the server is running and listening on the correct IP and port.
  2. Check that all devices are on the same network.
  3. Ensure firewalls or antivirus programs are not blocking the connection.

## Build and Run Command

### Using `build.sh` Script

To simplify the build process, a `build.sh` script is provided. This script will compile both the server and client programs.

```sh
./build.sh
```
