#!/bin/bash

# Probabilistic server

# Default Name: server-probabilistic
gcc -o server-probabilistic server-probabilistic.c -lws2_32
gcc -o start server-probabilistic.c -lws2_32

# Remove existing client executable if it exists
if [ -f client.exe ]; then
    rm client.exe
    rm join.exe
fi

# Compile client.c with ws2_32 library
gcc -o client client.c -lws2_32
gcc -o join client.c -lws2_32

echo "Build complete. Executables are smart-server and client."