#include "game_state.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#define PORT 8080

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    Player players[MAX_PLAYERS];
    int player_count = 0;
    Stack cardStack;
    int game_mode = 0;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_PLAYERS) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Waiting for connections...\n");

    // Accept the first player connection
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET) {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
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
            closesocket(clientSocket);
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        } else {
            // Enter player name for Player 1
            send(clientSocket, "Enter your name: ", 18, 0);
            bytesRead = recv(clientSocket, players[0].name, sizeof(players[0].name) - 1, 0);
            if (bytesRead > 0) {
                players[0].name[bytesRead] = '\0'; // Null-terminate the string
                printf("Player 1 name: %s\n", players[0].name);
            } else {
                printf("recv failed: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                closesocket(serverSocket);
                WSACleanup();
                return 1;
            }
        }

        int required_players = game_mode == 1 ? 1 : game_mode;

        // Accept additional player connections if needed
        for (int i = 1; i < required_players; i++) {
            clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
            if (clientSocket == INVALID_SOCKET) {
                printf("Accept failed. Error Code: %d\n", WSAGetLastError());
                closesocket(serverSocket);
                WSACleanup();
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
                printf("recv failed: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                closesocket(serverSocket);
                WSACleanup();
                return 1;
            }
        }

    } else {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Initialize and fill the card stack with a new seed
    srand(time(NULL)); // Use the current time as the seed for the random number generator
    resetAndFillStack(&cardStack);
    printStack(&cardStack);

    // Reset player states at the start of the game
    reset_player_states(players, player_count);

    // Initialize dealer
    Player dealer;
    dealer.hand_size = 0;
    dealer.score = 0;
    dealer.is_active = 1;

    // Deal initial cards
    deal_initial_cards(players, player_count, &dealer, &cardStack);

    // Game loop
    for (int i = 0; i < player_count; i++) {
        if (players[i].is_active) {
            display_player_cards(&players[i]);
            prompt_player_action(players, player_count, &players[i], &dealer, &cardStack);
        }
    }

    // Dealer's turn
    dealer_turn(&dealer, &cardStack);

    // Determine winners
    determine_winners(players, player_count, &dealer);

    // Cleanup
    for (int i = 0; i < player_count; i++) {
        closesocket(players[i].socket);
    }
    closesocket(serverSocket);
    WSACleanup();

    // Clean the stack at the end of the game
    cleanStack(&cardStack);

    return 0;
}