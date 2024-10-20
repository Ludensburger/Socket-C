#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096 // Increased buffer size
#define MAX_PLAYERS 4

// Define constants for the strings with colors
const char *DEALER_STRING = "\033[1;33mDealer:\t\033[0m";     // Yellow color
const char *PLAYER_STRING = "\033[1;34m Player %d:\t\033[0m"; // Blue color

typedef struct {
    SOCKET socket;
    char name[50];
    int hand[10]; // Array to hold cards
    int hand_size;
    int score;
    int is_active; // 1 if player is still in the game, 0 if busted or stood
} Player;

void shuffle_deck(int deck[]) {
    for (int i = 0; i < 52; i++) {
        deck[i] = i; // Fill the deck with values 0-51
    }
    srand(time(NULL)); // Seed random number generator
    for (int i = 0; i < 52; i++) {
        int j = rand() % 52; // Random index
        // Swap deck[i] and deck[j]
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

void deal_initial_cards(Player players[], int player_count, Player *dealer, int deck[], int *deck_index) {
    for (int i = 0; i < player_count; i++) {
        for (int j = 0; j < 2; j++) {
            players[i].hand[players[i].hand_size++] = deck[(*deck_index)++];
        }
    }
    // Deal two cards to the dealer
    for (int j = 0; j < 2; j++) {
        dealer->hand[dealer->hand_size++] = deck[(*deck_index)++];
    }
}

void display_player_cards(Player *player) {
    printf("Player %s's cards: ", player->name);
    for (int i = 0; i < player->hand_size; i++) {
        printf("%d ", player->hand[i]);
    }
    printf("\n");
}

void calculate_score(Player *player) {
    player->score = 0;
    int aces = 0;
    for (int i = 0; i < player->hand_size; i++) {
        int card_value = player->hand[i] % 13;
        if (card_value >= 10) {
            player->score += 10;
        } else if (card_value == 0) {
            player->score += 11;
            aces++;
        } else {
            player->score += card_value + 1;
        }
    }
    while (player->score > 21 && aces > 0) {
        player->score -= 10;
        aces--;
    }
}

const char *card_to_string(int card) {
    static char buffer[64]; // Increased buffer size
    const char *values[] = {"Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
    const char *suits[] = {"Spades", "Hearts", "Diamonds", "Clubs"};
    const char *colors[] = {"\033[38;5;208m", "\033[31m", "\033[36m", "\033[92m"}; // Orange, Red, Cyan, Light Green

    int value_index = card % 13;
    int suit_index = card / 13;

    snprintf(buffer, sizeof(buffer), "%s%s of %s\033[0m", colors[suit_index], values[value_index], suits[suit_index]);
    return buffer;
}

void send_game_state(Player players[], int player_count, Player *dealer) {
    char buffer[BUFFER_SIZE]; // Increased buffer size
    int offset = 0;

    // Add dealer's cards to the buffer
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", DEALER_STRING);
    for (int i = 0; i < dealer->hand_size; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
    }
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

    // Add each player's cards to the buffer
    for (int i = 0; i < player_count; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, PLAYER_STRING, i + 1);
        for (int j = 0; j < players[i].hand_size; j++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(players[i].hand[j]));
        }
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
    }

    // Null-terminate the buffer
    buffer[offset] = '\0';

    // Send the game state to all players
    for (int i = 0; i < player_count; i++) {
        send(players[i].socket, buffer, strlen(buffer), 0);
    }
}

void prompt_player_action(Player players[], int player_count, Player *player, Player *dealer, int deck[], int *deck_index) {
    char buffer[BUFFER_SIZE]; // Increased buffer size
    int bytesRead;

    while (player->is_active) {
        // Combine game state and prompt into a single message
        int offset = 0;
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", DEALER_STRING);
        for (int i = 0; i < dealer->hand_size; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
        }
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

        for (int i = 0; i < player_count; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, PLAYER_STRING, i + 1);
            for (int j = 0; j < players[i].hand_size; j++) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(players[i].hand[j]));
            }
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
        }

        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\nYour turn: hit or stand?\n");

        // Send the combined message to the player
        send(player->socket, buffer, strlen(buffer), 0);

        // Receive action from player
        bytesRead = recv(player->socket, buffer, BUFFER_SIZE, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            printf("Received from player: %s\n", buffer);

            if (strcmp(buffer, "hit") == 0) {
                // Deal a new card to the player
                player->hand[player->hand_size++] = deck[(*deck_index)++];
                calculate_score(player);

                // Combine updated game state and message
                offset = 0;
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", DEALER_STRING);
                for (int i = 0; i < dealer->hand_size; i++) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
                }
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

                for (int i = 0; i < player_count; i++) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, PLAYER_STRING, i + 1);
                    for (int j = 0; j < players[i].hand_size; j++) {
                        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(players[i].hand[j]));
                    }
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
                }

                // Check if player is busted
                if (player->score > 21) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "You are busted!\n");
                    player->is_active = 0;
                }

                // Send the updated game state and message to the player
                send(player->socket, buffer, strlen(buffer), 0);
            } else if (strcmp(buffer, "stand") == 0) {
                player->is_active = 0;
                send(player->socket, "You chose to stand.\n", 20, 0);
            } else {
                send(player->socket, "Invalid action. Please type 'hit' or 'stand'.\n", 45, 0);
            }
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
            player->is_active = 0;
        }
    }
}

void dealer_turn(Player *dealer, int deck[], int *deck_index) {
    while (dealer->score < 17) {
        dealer->hand[dealer->hand_size++] = deck[(*deck_index)++];
        calculate_score(dealer);
    }
}

void determine_winners(Player players[], int player_count, Player *dealer) {
    char buffer[BUFFER_SIZE]; // Increased buffer size
    int offset = 0;

    // Add dealer's cards to the buffer
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", DEALER_STRING);
    for (int i = 0; i < dealer->hand_size; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
    }
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

    // Add each player's cards to the buffer
    for (int i = 0; i < player_count; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, PLAYER_STRING, i + 1);
        for (int j = 0; j < players[i].hand_size; j++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(players[i].hand[j]));
        }
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
    }

    // Null-terminate the buffer
    buffer[offset] = '\0';

    // Send the final game state to all players
    for (int i = 0; i < player_count; i++) {
        send(players[i].socket, buffer, strlen(buffer), 0);
    }

    // Determine and send the result to each player
    for (int i = 0; i < player_count; i++) {
        if (players[i].score > 21) {
            send(players[i].socket, "\033[31mYou lost!\033[0m\n", 20, 0); // Red color
        } else if (dealer->score > 21 || players[i].score > dealer->score) {
            send(players[i].socket, "\033[36mYou won!\033[0m\n", 19, 0); // Cyan color
        } else if (players[i].score == dealer->score) {
            send(players[i].socket, "It's a tie!\n", 12, 0);
        } else {
            send(players[i].socket, "\033[31mYou lost!\033[0m\n", 20, 0); // Red color
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    Player players[MAX_PLAYERS];
    int player_count = 0;
    int deck[52];
    int deck_index = 0;
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

    players[player_count].socket = clientSocket;
    players[player_count].hand_size = 0;
    players[player_count].score = 0;
    players[player_count].is_active = 1;
    player_count++;
    printf("Player 1 connected.\n");

    // Prompt the first player to choose the game mode
    send(clientSocket, "\033[1;31mChoose game mode:\033[0m \033[1;32m1-PvE\033[0m, \033[1;33m2-1v1\033[0m, \033[1;34m3-1v2\033[0m, \033[1;35m4-1v3\033[0m, \033[1;36m5-1v4\033[0m\n", 100, 0);
    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the string
        game_mode = atoi(buffer);
        printf("Game mode selected: %d\n", game_mode);
    } else {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Determine the number of players needed based on the game mode
    int required_players = game_mode == 1 ? 1 : game_mode;

    // Accept additional player connections if needed
    while (player_count < required_players) {
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
        player_count++;
        printf("Player %d connected.\n", player_count);
    }

    // Shuffle deck
    shuffle_deck(deck);

    // Initialize dealer
    Player dealer;
    dealer.hand_size = 0;
    dealer.score = 0;
    dealer.is_active = 1;

    // Deal initial cards
    deal_initial_cards(players, player_count, &dealer, deck, &deck_index);

    // Game loop
    for (int i = 0; i < player_count; i++) {
        if (players[i].is_active) {
            display_player_cards(&players[i]);
            prompt_player_action(players, player_count, &players[i], &dealer, deck, &deck_index);
        }
    }

    // Dealer's turn
    dealer_turn(&dealer, deck, &deck_index);

    // Determine winners
    determine_winners(players, player_count, &dealer);

    // Cleanup
    for (int i = 0; i < player_count; i++) {
        closesocket(players[i].socket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}