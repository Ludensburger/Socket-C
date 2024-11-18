- `client.c`: The client-side code for connecting to the Blackjack server.
- `server.c`: The server-side code for managing the Blackjack game.

## How to Build

1. Make sure you have a C compiler installed (e.g., `gcc`).
2. Open a terminal and navigate to the `BlackJack` directory.
3. Compile the server and client code:

```sh
gcc -o server server.c  -lws2_32
gcc -o client client.c -lws2_32
```
