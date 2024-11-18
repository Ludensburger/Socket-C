#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status
set -x  # Print each command before it is executed

# Compile each source file into an object file
gcc -c utils.c -o utils.o
gcc -c client.c -o client.o
gcc -c server.c -o server.o

# Link object files to create the server executable
gcc server.o utils.o -o server.exe -lws2_32

# Link object files to create the client executable
gcc client.o utils.o -o client.exe -lws2_32

echo "Build complete. Executables are server.exe and client.exe."