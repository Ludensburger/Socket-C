#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096 // Increased buffer size
#define MAX_PLAYERS 4
#define STACK_SIZE 52    // Use a full deck of 52 cards
#define GAMEMASTER "Ryu" // Name of the game master

// Define constants for the strings with colors
const char *DEALER_STRING = "\033[1;33mDealer:\033[0m"; // Yellow color
int debugCounter = 1;

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

    printf("Shuffling the deck");
    for (int i = 0; i < 3; i++) {
        printf(".");
        Sleep(500); // Wait 500ms.
    }
    printf("\n");

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

const char *card_to_string(int card) {
    static char buffers[4][64]; // Use an array of buffers to avoid overwriting
    static int buffer_index = 0;

    const char *values[] = {"Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
    const char *suits[] = {"Spades", "Hearts", "Diamonds", "Clubs"};
    const char *colors[] = {"\033[38;5;208m", "\033[31m", "\033[36m", "\033[92m"}; // Orange, Red, Cyan, Light Green

    int value_index = card % 13;
    int suit_index = card / 13;

    // Use a different buffer each time
    char *buffer = buffers[buffer_index];
    buffer_index = (buffer_index + 1) % 4;

    snprintf(buffer, 64, "%s%s of %s\033[0m", colors[suit_index], values[value_index], suits[suit_index]);
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

void print_debug_info(Player *dealer, Player players[], int player_count) {
    printf("\n----------- Debug Info %d -----------\n", debugCounter++);

    // Show dealer's cards and score
    printf("%s\n", DEALER_STRING); // Use the yellow colored dealer string
    printf("Score: %d\n", dealer->score);
    printf("Hand: ");
    for (int i = 0; i < dealer->hand_size; i++) {
        printf("%s", card_to_string(dealer->hand[i]));
        if (i < dealer->hand_size - 1) {
            printf(", ");
        }
    }
    printf("\n\n");

    // Show all players' cards
    for (int i = 0; i < player_count; i++) {
        printf("%s%s:\033[0m\n", players[i].color, players[i].name);
        printf("Score: %d\n", players[i].score);
        printf("Hand: ");
        for (int j = 0; j < players[i].hand_size; j++) {
            printf("%s", card_to_string(players[i].hand[j]));
            if (j < players[i].hand_size - 1) {
                printf(", ");
            }
        }
        printf("\n\n");
    }
    printf("------------------------------------\n\n");
}

void calculate_score(Player *player, Player players[], int player_count, Player *dealer) {

    player->score = 0;
    int aces = 0;

    // Calculate the initial score and count the number of Aces
    for (int i = 0; i < player->hand_size; i++) {
        int card_value = player->hand[i] % 13;

        // If face card (Jack, Queen, King), add 10 to the score
        if (card_value >= 10) {
            player->score += 10;
        }
        // If Ace, initially add 11 to the score and increment the Ace count
        else if (card_value == 0) {
            player->score += 11;
            aces++;
        }
        // For other cards (2 to 9), add their value to the score
        else {
            player->score += card_value + 1;
        }
    }

    // Adjust the score if it exceeds 21 by counting some Aces as 1 instead of 11
    while (player->score > 21 && aces > 0) {
        player->score -= 10;
        aces--;
    }
}

void deal_initial_cards(Player players[], int player_count, Player *dealer, Stack *cardStack) {
    const char *suits[] = {"Spades", "Hearts", "Diamonds", "Clubs"};

    printf("Dealing cards");
    for (int i = 0; i < player_count + 2; i++) {
        printf(".");
        Sleep(500); // Wait 500ms.
    }
    printf("\n");

    for (int i = 0; i < player_count; i++) {
        players[i].hand_size = 0; // Clear player hands

        // Check if the player is named GAMEMASTER or Tester
        if (strcmp(players[i].name, GAMEMASTER) == 0 || strcmp(players[i].name, "Tester") == 0) {
            // Assign a random Ace and a random 10-value card
            int suit_index = rand() % 4;
            int ace_card = suit_index * 13;                             // Ace of the chosen suit
            int ten_value_card = (suit_index * 13) + 10 + (rand() % 4); // 10, Jack, Queen, King of the same suit

            players[i].hand[players[i].hand_size++] = ace_card;
            players[i].hand[players[i].hand_size++] = ten_value_card;
        } else {
            // Deal two random cards to the player
            for (int j = 0; j < 2; j++) {
                players[i].hand[players[i].hand_size++] = pop(cardStack);
            }
        }
    }

    // Deal two cards to the dealer
    dealer->hand_size = 0; // Clear dealer hand
    for (int j = 0; j < 2; j++) {
        dealer->hand[dealer->hand_size++] = pop(cardStack);
    }

    // Calculate players' initial scores after dealing cards
    for (int i = 0; i < player_count; i++) {
        calculate_score(&players[i], players, player_count, dealer);
    }

    // Calculate dealer's score last
    calculate_score(dealer, players, player_count, dealer);
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

void printStack(Stack *stack, int player_count) {

    // Seed the random number generator
    srand(time(NULL));

    // Adjust the division of cards in the server if needed
    // can be set to be dynamic based on the number of players
    // Example:
    int cardPartition = player_count;
    // int cardPartition = 2;

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

// Helper method to convert a string to lowercase
void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
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

        // Add the player's own cards to the buffer
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s%s:\t\033[0m", player->color, player->name);
        for (int j = 0; j < player->hand_size; j++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(player->hand[j]));
        }
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\nYour turn: hit or stand?\nEnter your action (hit/stand): ");

        // Send the combined message to the player
        calculate_score(player, players, player_count, dealer); // Calculate score before prompting for action
        send(player->socket, buffer, strlen(buffer), 0);

        // Receive action from player
        bytesRead = recv(player->socket, buffer, BUFFER_SIZE, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            printf("Received from %s: %s\n", player->name, buffer);

            // Convert input to lowercase
            to_lowercase(buffer);

            if (strcmp(buffer, "hit") == 0) {
                // Deal a new card to the player
                if (isEmpty(cardStack)) {
                    fillStack(cardStack);
                }
                player->hand[player->hand_size++] = pop(cardStack);
                calculate_score(dealer, players, player_count, dealer); // Add this line
                calculate_score(player, players, player_count, dealer);
                print_debug_info(dealer, players, player_count);

                // Combine updated game state and message
                offset = 0;
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s", DEALER_STRING);
                for (int i = 0; i < dealer->hand_size; i++) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(dealer->hand[i]));
                }
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

                // Add the player's own cards to the buffer
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, " %s%s:\t\033[0m", player->color, player->name);
                for (int j = 0; j < player->hand_size; j++) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, " | %s", card_to_string(player->hand[j]));
                }
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

                // Check if player is busted
                if (player->score > 21) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "You are busted!\n");
                    player->is_active = 0;
                }

                // Send the updated game state and message to the player
                send(player->socket, buffer, strlen(buffer), 0);
            } else if (strcmp(buffer, "stand") == 0) {
                player->is_active = 0;
                calculate_score(dealer, players, player_count, dealer); // Add this line
                calculate_score(player, players, player_count, dealer);
                print_debug_info(dealer, players, player_count);

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

void dealer_turn(Player *dealer, Stack *cardStack, Player players[], int player_count) {
    while (dealer->score < 17) {
        if (isEmpty(cardStack)) {
            fillStack(cardStack);
        }
        dealer->hand[dealer->hand_size++] = pop(cardStack);
        calculate_score(dealer, players, player_count, dealer);
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

    // Add final scores to the buffer
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\nFinal Scores:\n");

    // Determine color for dealer's score
    const char *dealer_score_color;

    // Exactly 21 points is a winning score
    // Green color
    if (dealer->score == 21) {
        dealer_score_color = "\033[32m";
    }

    // Close to 21 points is a good score
    // Cyan color
    else if (dealer->score >= 19 && dealer->score < 21) {
        dealer_score_color = "\033[36m";
    }

    // Less than 19 points is a losing score
    // Orange color
    else if (dealer->score < 19) {
        dealer_score_color = "\033[33m";
    }

    // More than 21 points is a bust
    else {
        dealer_score_color = "\033[31m";
    }
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s %s%d\033[0m\n", DEALER_STRING, dealer_score_color, dealer->score);

    // Determine color for each player's score
    for (int i = 0; i < player_count; i++) {
        const char *score_color;

        // Exactly 21 points is a winning score
        // Green
        if (players[i].score == 21) {
            score_color = "\033[32m";
        }

        // Close to 21 points is a good score
        // Cyan
        else if (players[i].score >= 19 && players[i].score < 21) {
            score_color = "\033[36m";
        }

        // Less than 19 points is a losing score
        // Orange
        else if (players[i].score < 19) {
            score_color = "\033[33m";
        }

        // More than 21 points is a bust
        // Red
        else {
            score_color = "\033[31m";
        }

        // Player name and score with colors
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s%s: %s%d\033[0m\n", players[i].color, players[i].name, score_color, players[i].score);
    }

    // Null-terminate the buffer
    buffer[offset] = '\0';

    // Send the final game state to all players
    for (int i = 0; i < player_count; i++) {
        send(players[i].socket, buffer, strlen(buffer), 0);
    }

    // Determine if the dealer wins
    if (dealer->score == 21) {
        // Dealer wins, all players lose
        for (int i = 0; i < player_count; i++) {
            send(players[i].socket, "\033[31mDealer wins! You lost!\033[0m\n", 38, 0); // Red color
        }
    } else {
        // Determine the closest player to 21
        int best_score = 0;
        int winner_index = -1;
        for (int i = 0; i < player_count; i++) {

            // Whoever gets 21 first automatically wins, especially Ace + 10 or face card
            if (players[i].score == 21) {
                best_score = 21;
                winner_index = i;
                break;
            }

            if (players[i].score <= 21 && players[i].score > best_score) {
                best_score = players[i].score;
                winner_index = i;
            }
        }

        // Send the result to each player
        for (int i = 0; i < player_count; i++) {
            if (i == winner_index) {
                send(players[i].socket, "\033[36mYou won!\033[0m\n", 19, 0); // Cyan color
            } else {
                // Show who won with name colored
                char message[BUFFER_SIZE];
                snprintf(message, sizeof(message), "%s%s won!\033[0m\n", players[winner_index].color, players[winner_index].name);
                send(players[i].socket, message, strlen(message), 0); // Winner's name in their color

                // Send "Better luck next time" message in yellow
                send(players[i].socket, "\033[33mBetter luck next time.\033[0m\n", 32, 0); // Yellow color
            }
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
    const char *game_mode_prompt =
        "\033[1;31mChoose game mode:\033[0m\n"
        "    \033[1;32m1. PvE (Player vs Environment)\033[0m\n"
        "    \033[1;33m2. 1v1 (Player vs Player)\033[0m\n"
        "    \033[1;34m3. 1v2 (Player vs 2 Players)\033[0m\n"
        "    \033[1;35m4. 1v3 (Player vs 3 Players)\033[0m\n"
        "    \033[1;36m5. 1v4 (Player vs 4 Players)\033[0m\n\n"
        "  Enter game mode ( \033[1;32m1\033[0m | \033[1;33m2\033[0m | \033[1;34m3\033[0m | \033[1;35m4\033[0m | \033[1;36m5\033[0m ): ";
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
            send(clientSocket, "Enter your name (#1): ", 23, 0);
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
            char name_prompt[50];
            snprintf(name_prompt, sizeof(name_prompt), "Enter your name (#%d): ", i + 1);
            send(clientSocket, name_prompt, strlen(name_prompt), 0);
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
    print_debug_info(&dealer, players, player_count);

    // Game loop
    for (int i = 0; i < player_count; i++) {
        if (players[i].is_active) {
            display_player_cards(&players[i]);
            prompt_player_action(players, player_count, &players[i], &dealer, &cardStack);
        }
    }

    // Dealer's turn
    dealer_turn(&dealer, &cardStack, players, player_count);
    print_debug_info(&dealer, players, player_count); // Add here

    // Determine winners
    print_debug_info(&dealer, players, player_count); // Add here
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
