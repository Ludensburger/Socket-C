#!/bin/bash

# Compile server.c with ws2_32 library
# Default server
gcc -o server server.c -lws2_32

# Easy server
# Default Name: server-easy
gcc -o server-easy server-easy.c -lws2_32

# Probabilistic server
# Default Name: server-probabilistic
gcc -o server-probabilistic server-probabilistic.c -lws2_32

# Easy Mode
gcc -o easy server-easy.c -lws2_32

# Probabilistic Mode
gcc -0 smart server-probabilistic.c -lws2_32


# Compile client.c with ws2_32 library
gcc -o client client.c -lws2_32

echo "Build complete. Executables are server and client."