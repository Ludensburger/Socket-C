#!/bin/bash

# Compile addressFinder.c with iphlpapi library
gcc addressFinder.c -liphlpapi -o addressFinder

# Compile server.c with ws2_32 library
# Default server
gcc -o server server.c -lws2_32


# Remove existing client executable if it exists
if [ -f client.exe ]; then
    rm client.exe
fi

# Compile client.c with ws2_32 library
gcc -o client client.c -lws2_32

echo "Build complete. Executables are server and client."