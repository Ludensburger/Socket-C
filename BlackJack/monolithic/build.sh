#!/bin/bash

# Compile addressFinder.c with iphlpapi library
gcc addressFinder.c -liphlpapi -o addressFinder

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
gcc -o smart server-probabilistic.c -lws2_32

# Remove existing client executable if it exists
if [ -f client.exe ]; then
    rm client.exe
fi

# Compile client.c with ws2_32 library
gcc -o client client.c -lws2_32

echo "Build complete. Executables are server and client."