#!/bin/bash

# Compile server.c with ws2_32 library
gcc -o server server.c -lws2_32

# Compile client.c with ws2_32 library
gcc -o client client.c -lws2_32

echo "Build complete. Executables are server and client."