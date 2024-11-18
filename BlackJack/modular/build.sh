#!/bin/bash

# Compile each source file into an object file
gcc -c player.c -o player.o
gcc -c stack.c -o stack.o
gcc -c utils.c -o utils.o
gcc -c game_state.c -o game_state.o
gcc -c server.c -o server.o
gcc -c client.c -o client.o

# Link all object files to create the server executable
gcc player.o stack.o utils.o game_state.o server.o -o server.exe -lws2_32

# Link all object files to create the client executable
gcc player.o stack.o utils.o game_state.o client.o -o client.exe -lws2_32