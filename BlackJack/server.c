#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096 // Increased buffer size
#define MAX_PLAYERS 4
#define STACK_SIZE 52 // Use a full deck of 52 cards

// Define constants for the strings with colors
const char *DEALER_STRING = "\033[1;33mDealer:\033[0m"; // Yellow color

typedef struct {
    SOCKET socket;
    char name[50];
    int hand[10]; // Array to hold cards
    int hand_size;
    int score;
    int is_active;  // 1 if player is still in the game, 0 if busted or stood
    char color[10]; // Color code for the player
} Player;

typedef struct {
    int cards[STACK_SIZE];
    int top;
} Stack;

void initializeStack(Stack *stack) {
    stack->top = -1;
}

int isEmpty(Stack *stack) {
    return stack->top == -1;
}

int isFull(Stack *stack) {
    return stack->top == STACK_SIZE - 1;
}

void push(Stack *stack, int card) {
    if (isFull(stack)) {
        printf("Stack is full. Cannot push card %d\n", card);
        return;
    }
    stack->cards[++stack->top] = card;
}

int pop(Stack *stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty. Cannot pop card\n");
        return -1;
    }
    return stack->cards[stack->top--];
}

void shuffleCards(int *cards, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }
}

void fillStack(Stack *stack) {
    int cards[STACK_SIZE];
    for (int i = 0; i < STACK_SIZE; i++) {
        cards[i] = i; // Generate card values from 0 to 51
    }
    shuffleCards(cards, STACK_SIZE);
    initializeStack(stack);
    for (int i = 0; i < STACK_SIZE; i++) {
        push(stack, cards[i]);
    }
}

void resetAndFillStack(Stack *stack) {
    int cards[STACK_SIZE];
    for (int i = 0; i < STACK_SIZE; i++) {
        cards[i] = i; // Generate card values from 0 to 51
    }
    shuffleCards(cards, STACK_SIZE);
    initializeStack(stack);
    for (int i = 0; i < STACK_SIZE; i++) {
        push(stack, cards[i]);
    }
}

void deal_initial_cards(Player players[], int player_count, Player *dealer, Stack *cardStack) {
    for (int i = 0; i < player_count; i++) {
        players[i].hand_size = 0; // Clear player hands
        for (int j = 0; j < 2; j++) {
            players[i].hand[players[i].hand_size++] = pop(cardStack);
        }
    }
    // Deal two cards to the dealer
    dealer->hand_size = 0; // Clear dealer hand
    for (int j = 0; j < 2; j++) {
        dealer->hand[dealer->hand_size++] = pop(cardStack);
    }
}

void cleanStack(Stack *stack) {
    initializeStack(stack);
}

void display_player_cards(Player *player) {
    printf("Player %s's cards: ", player->name);
    for (int i = 0; i < player->hand_size; i++) {
        printf("%d ", player->hand[i]);
    }
    printf("\n");
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

// Function to generate a random color code
const char *getRandomColor() {
    const char *colors[] = {
        "\033[31m", // Red
        "\033[32m", // Green
        "\033[33m", // Yellow
        "\033[34m", // Blue
        "\033[35m", // Magenta
        "\033[36m", // Cyan
    };
    int num_colors = sizeof(colors) / sizeof(colors[0]);
    return colors[rand() % num_colors];
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

    // Improved debugging statements to trace the score calculation
    printf("\n--- Debug Info ---\n");
    printf("Player: %s\n", player->name);
    printf("Score: %d\n", player->score);
    printf("Hand: ");
    for (int i = 0; i < player->hand_size; i++) {
        printf("%s", card_to_string(player->hand[i]));
        if (i < player->hand_size - 1) {
            printf(", "); // Add a comma between cards
        }
    }
    printf("\n------------------\n\n");
}

void printStack(Stack *stack) {
    srand(time(NULL));     // Seed the random number generator
    int cardPartition = 2; // Adjust the division of cards in the server if needed
    for (int i = 0; i <= stack->top; i++) {
        if (i % 10 == 0 && i != 0) {
            printf("\n"); // Print a blank line for a new stack
        }
        if (i % cardPartition == 0) {
            printf("-------Stack %d-------\n", (i / 10) + 1); // Stack Bar
        }
        printf("%s\n", card_to_string(stack->cards[i])); // Cards
    }
    printf("\n");
}

void send_game_state(Player players[], int player_count, Player *dealer) {
    char buffer[BUFFER_SIZE]; // Increased buffer size
    int offset = 0;

    // Add dealer's cards to the buffer
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s", DEALER_STRING);
    for (int i = 0; i < dealer->hand_size; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
    }
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

    // Add each player's cards to the buffer
    for (int i = 0; i < player_count; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s%s:\t\033[0m", players[i].color, players[i].name);
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

void prompt_player_action(Player players[], int player_count, Player *player, Player *dealer, Stack *cardStack) {
    char buffer[BUFFER_SIZE]; // Increased buffer size
    int bytesRead;

    while (player->is_active) {
        // Combine game state and prompt into a single message
        int offset = 0;
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s", DEALER_STRING);
        for (int i = 0; i < dealer->hand_size; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
        }
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

        for (int i = 0; i < player_count; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s%s:\t\033[0m", players[i].color, players[i].name);
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
            printf("Received from %s: %s\n", player->name, buffer);

            if (strcmp(buffer, "hit") == 0) {
                // Deal a new card to the player
                if (isEmpty(cardStack)) {
                    fillStack(cardStack);
                }
                player->hand[player->hand_size++] = pop(cardStack);
                calculate_score(player); // Calculate score after dealing a new card

                // Combine updated game state and message
                offset = 0;
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s", DEALER_STRING);
                for (int i = 0; i < dealer->hand_size; i++) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
                }
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

                for (int i = 0; i < player_count; i++) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s%s:\t\033[0m", players[i].color, players[i].name);
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

void dealer_turn(Player *dealer, Stack *cardStack) {
    while (dealer->score < 17) {
        if (isEmpty(cardStack)) {
            fillStack(cardStack);
        }
        dealer->hand[dealer->hand_size++] = pop(cardStack);
        calculate_score(dealer);
    }
}

void determine_winners(Player players[], int player_count, Player *dealer) {
    char buffer[BUFFER_SIZE]; // Increased buffer size
    int offset = 0;

    // Add dealer's cards to the buffer
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s", DEALER_STRING);
    for (int i = 0; i < dealer->hand_size; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
    }
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

    // Add each player's cards to the buffer
    for (int i = 0; i < player_count; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s%s:\t\033[0m", players[i].color, players[i].name);
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

void reset_player_states(Player players[], int player_count) {
    for (int i = 0; i < player_count; i++) {
        players[i].hand_size = 0;
        players[i].score = 0;
        players[i].is_active = 1;
        strcpy(players[i].color, getRandomColor()); // Reassign random color to each player
    }
}

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