# Socket-C

## Overview

Socket-C is a repository designed for learning socket programming in C, using the Winsock2 library. It demonstrates network communication through two multiplayer games: Blackjack and Rock-Paper-Scissors. By simulating interactions across multiple computers, Socket-C provides a hands-on approach to understanding socket programming concepts.

## Features

- Multiplayer support for up to 4 players
- Real-time game state updates between clients
- Color-coded terminal output for improved readability
- Practical demonstrations of essential socket programming concepts

## Games

- Blackjack ♠♥
- Rock-Paper-Scissors 🪨📃✂

## Prerequisites

- **Windows OS**
- **Microsoft Visual Studio** or any C compiler supporting Winsock2 (e.g., GCC with MinGW)
- Basic understanding of C and network programming concepts

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

## Template Directory

I've also added a `template` directory for you to create your own socket programming projects. This serves as your starting point or boilerplate in C socket programming.

## Notes

- Ensure your firewall allows communication for the server and client programs to work correctly.
- All players should be on the same network unless port forwarding is configured for remote connections.
