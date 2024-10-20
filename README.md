# Socket-C

## Overview

Socket-C is a repository for learning socket programming in C using the Winsock2 library. It demonstrates network communication through two games: Blackjack and Rock-Paper-Scissors, simulating multiplayer interactions across different computers. This repository offers a practical introduction to socket programming by providing hands-on examples of networking concepts in action.

## Features

- Multiplayer support for up to 4 players
- Multiple game modes for Blackjack: Player vs Environment (PvE), 1v1, 1v2, 1v3, 1v4
- Real-time game state updates between clients
- Color-coded terminal output for improved readability
- Practical demonstrations of core socket programming concepts

## Prerequisites

- **Windows OS**
- **Microsoft Visual Studio** or any C compiler with Winsock2 support (e.g., GCC with MinGW)
- Basic understanding of C programming and networking concepts

## Setup Instructions

1. **Clone the Repository:**

   ```sh
   git clone https://github.com/yourusername/Socket-C.git
   cd Socket-C
   ```

2. **Build the Files:**

   For Windows (using GCC):

   ```sh
   gcc -o server server.c -lws2_32
   gcc -o client client.c -lws2_32
   ```

3. **Run the Executables:**

   - **For Command Prompt:**

     ```sh
     ./server
     ./client
     ```

   - **For PowerShell:**
     ```sh
     ./server
     ./client
     ```

## Notes

- Ensure your firewall allows communication for the server and client programs to work properly.
- All players must be on the same network, or you may need to configure port forwarding for remote connections.

This revision adds clarity, consistency, and useful details (like the note about firewalls) while maintaining the concise structure.
