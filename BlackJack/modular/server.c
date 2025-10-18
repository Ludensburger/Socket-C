#include "game_state.h"
#include "utils.h"
#include "server_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8080

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    Player players[MAX_PLAYERS];
    int player_count = 0;
    Stack cardStack;
    int game_mode = 0;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Allow the socket to be reused immediately after it's closed
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(serverSocket);
        return 1;
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_PLAYERS) < 0) {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for connections...\n");

    // Accept the first player connection
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientSocket < 0) {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    // When accepting the first player connection
    players[player_count].socket = clientSocket;
    players[player_count].hand_size = 0;
    players[player_count].score = 0;
    players[player_count].is_active = 1;

    // Assign a random color to the player
    const char *playerColor = getRandomColor();
    strcpy(players[player_count].color, playerColor); // Assign random color to Player 1
    player_count++;
    printf("Player 1 connected.\n");

    // Prompt the first player to choose the game mode
    const char *game_mode_prompt = "\033[1;31mChoose game mode:\033[0m \033[1;32m1-PvE\033[0m, \033[1;33m2-1v1\033[0m, \033[1;34m3-1v2\033[0m, \033[1;35m4-1v3\033[0m, \033[1;36m5-1v4\033[0m\n";
    send(clientSocket, game_mode_prompt, strlen(game_mode_prompt), 0);

    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the string
        game_mode = atoi(buffer);
        printf("Game mode selected: %d\n", game_mode);

        // Validate the game mode
        if (game_mode < 1 || game_mode > 5) {
            printf("Invalid game mode. Exiting...\n");
            close(clientSocket);
            close(serverSocket);
            return 1;
        } else {
            // Enter player name for Player 1
            send(clientSocket, "Enter your name: ", 18, 0);
            bytesRead = recv(clientSocket, players[0].name, sizeof(players[0].name) - 1, 0);
            if (bytesRead > 0) {
                players[0].name[bytesRead] = '\0'; // Null-terminate the string
                printf("Player 1 name: %s\n", players[0].name);
            } else {
                perror("recv failed for player 1 name");
                close(clientSocket);
                close(serverSocket);
                return 1;
            }
        }

        int required_players = game_mode == 1 ? 1 : game_mode;

        // Accept additional player connections if needed
        for (int i = 1; i < required_players; i++) {
            clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
            if (clientSocket < 0) {
                perror("Accept failed for additional player");
                close(serverSocket);
                return 1;
            }

            players[player_count].socket = clientSocket;
            players[player_count].hand_size = 0;
            players[player_count].score = 0;
            players[player_count].is_active = 1;

            const char *otherPlayerColor = getRandomColor();

            while (playerColor == otherPlayerColor) {
                otherPlayerColor = getRandomColor();
            }

            strcpy(players[player_count].color, otherPlayerColor); // Assign random color to additional players
            player_count++;
            printf("Player %d connected.\n", i + 1);

            // Enter player name for additional players
            send(clientSocket, "Enter your name: ", 18, 0);
            bytesRead = recv(clientSocket, players[player_count - 1].name, sizeof(players[player_count - 1].name) - 1, 0);
            if (bytesRead > 0) {
                players[player_count - 1].name[bytesRead] = '\0'; // Null-terminate the string
                printf("Player %d name: %s\n", i + 1, players[player_count - 1].name);
            } else {
                perror("recv failed for additional player name");
                close(clientSocket);
                close(serverSocket);
                return 1;
            }
        }

    } else {
        perror("recv failed for game mode");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    int play_again = 1;
    while (play_again) {
        // Initialize and fill the card stack with a new seed
        srand(time(NULL)); // Use the current time as the seed for the random number generator
        resetAndFillStack(&cardStack);
        printStack(&cardStack, player_count);

        // Reset player states at the start of the game
        reset_player_states(players, player_count);

        // Initialize dealer
        Player dealer;
        dealer.hand_size = 0;
        dealer.score = 0;
        dealer.is_active = 1;

        // Deal initial cards
        deal_initial_cards(players, player_count, &dealer, &cardStack);

        // Print the deck state *after* initial cards have been dealt for an accurate log
        printStack(&cardStack, player_count);

        // Send only the "NEW_ROUND" signal to all clients
        for (int i = 0; i < player_count; i++) {
            send(players[i].socket, "NEW_ROUND", 9, 0);
        }

        // Player turns
        for (int i = 0; i < player_count; i++) {
            if (players[i].is_active) {
                // Display the dynamic server-side game state
                display_server_gamestate(&players[i], &cardStack);
                prompt_player_action(players, player_count, &players[i], &dealer, &cardStack);
            }
        }

        // Dealer's turn
        dealer_turn(&dealer, &cardStack, players, player_count);
        print_debug_info(&dealer, players, player_count); // Add final debug info after dealer's turn

        // Determine winners
        determine_winners(players, player_count, &dealer);

        bytesRead = recv(players[0].socket, buffer, BUFFER_SIZE, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            if (strcmp(buffer, "no") == 0) {
                play_again = 0;
            }
        } else {
            play_again = 0; // End game if player 1 disconnects
        }
    }

    // Cleanup
    for (int i = 0; i < player_count; i++) {
        close(players[i].socket);
    }
    close(serverSocket);

    // Clean the stack at the end of the game
    cleanStack(&cardStack);

    return 0;
}