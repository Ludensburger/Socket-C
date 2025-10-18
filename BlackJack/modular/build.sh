#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "Compiling server..."
# Compile all server-side .c files into a 'server' executable
# The -lm flag links the math library, which can be useful.
gcc -o server server.c game_state.c player.c stack.c utils.c server_utils.c -lm

echo "Compiling client..."
# Compile client-side .c files into a 'client' executable
gcc -o client client.c utils.c -lm

echo "Build complete."
echo "Run './server' in one terminal and './client' in another."