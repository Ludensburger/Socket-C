#include "utils.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 4096      // Increased buffer size
#define SERVER_IP "127.0.0.1" // Use this for local play

// --- Client-Side Game State ---
typedef struct
{
    char name[50];
    char hand_str[256];
    int score;
    int wallet;
    char color[20]; // Increased from 10 to 20 to handle longer ANSI codes
} ClientPlayer;

char dealer_hand_str[256];
int dealer_score;
ClientPlayer players[MAX_PLAYERS];
int player_count = 0;
int my_player_index = -1;
// --- End Client-Side Game State ---

// Helper function to convert a string to lowercase for case-insensitive comparison
void to_lowercase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}

// Function to get and validate user input for actions
void get_validated_input(char *buffer, size_t size)
{
    while (1)
    {
        fgets(buffer, size, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
        to_lowercase(buffer);

        if (strcmp(buffer, "hit") == 0 || strcmp(buffer, "stand") == 0)
        {
            return; // Valid input
        }
        printf("Invalid input. Please enter 'hit' or 'stand': ");
    }
}

void display_game_table()
{
    printf("\033[2J\033[H"); // Clear screen and move cursor to top-left

    // Add some top padding for better visual balance
    printf("\n");

    // Display table header with better formatting
    printf("\033[1;36m╔════════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║                        🎰  BLACKJACK TABLE  🎰                     ║\033[0m\n");
    printf("\033[1;36m╚════════════════════════════════════════════════════════════════════╝\033[0m\n\n");

    // Display Dealer with better formatting
    printf("\033[1;33m┌─ DEALER ───────────────────────────────────────────────────────────┐\033[0m\n");
    if (dealer_hand_str[0] != '\0')
    {
        printf("\033[1;33m│\033[0m  Hand: %s\n", dealer_hand_str);
        // Only show dealer score if revealed (score > 0 means cards are revealed)
        if (dealer_score > 0)
        {
            printf("\033[1;33m│\033[0m  Score: \033[1;37m%d\033[0m\n", dealer_score);
        }
        else
        {
            printf("\033[1;33m│\033[0m  Score: \033[1;30m???\033[0m\n");
        }
    }
    else
    {
        printf("\033[1;33m│\033[0m  Waiting...\n");
    }
    printf("\033[1;33m└────────────────────────────────────────────────────────────────────┘\033[0m\n\n");

    // Display Players section
    printf("\033[1;36m┌─ PLAYERS ──────────────────────────────────────────────────────────┐\033[0m\n");
    for (int i = 0; i < player_count; i++)
    {
        // Highlight player name with bold and their assigned color
        printf("\033[1;36m│\033[0m  \033[1m%s%s\033[0m\n", players[i].color, players[i].name);
        printf("\033[1;36m│\033[0m    Cards: %s\n", players[i].hand_str);
        // Don't show score - let players calculate it themselves!
        printf("\033[1;36m│\033[0m    Wallet: \033[1;32m$%d\033[0m\n", players[i].wallet);
        if (i < player_count - 1)
        {
            printf("\033[1;36m│\033[0m  ─────────────────────────────────────────────────────────────────\n");
        }
    }
    printf("\033[1;36m└────────────────────────────────────────────────────────────────────┘\033[0m\n\n");
}

void parse_and_update_state(char *server_message, int clientSocket)
{
    char temp_buffer[BUFFER_SIZE];

    if (strncmp(server_message, "STATE;", 6) == 0)
    {
        // DEBUG: Print what we received
        // fprintf(stderr, "=== CLIENT RECEIVED ===\n%s\n=======================\n", server_message);

        char message_copy[BUFFER_SIZE];
        strcpy(message_copy, server_message); // Work on a copy to avoid corrupting the main buffer
        player_count = 0;
        char *state_data = message_copy + 6;
        char *saveptr1; // Save pointer for outer strtok_r
        char *entity = strtok_r(state_data, "|", &saveptr1);
        while (entity != NULL)
        {
            char *saveptr2; // Save pointer for inner strtok_r
            char *type = strtok_r(entity, ";", &saveptr2);
            if (type == NULL)
            {
                // Do nothing if the token is empty
            }
            else if (strcmp(type, "DEALER") == 0)
            {
                char *hand_tok = strtok_r(NULL, ";", &saveptr2);
                if (hand_tok)
                    strcpy(dealer_hand_str, hand_tok);
                else
                    dealer_hand_str[0] = '\0';

                char *score_tok = strtok_r(NULL, ";", &saveptr2);
                if (score_tok)
                    dealer_score = atoi(score_tok);
                else
                    dealer_score = 0;
            }
            else if (strcmp(type, "PLAYER") == 0)
            {
                char *name_tok = strtok_r(NULL, ";", &saveptr2);
                if (name_tok)
                    strcpy(players[player_count].name, name_tok);
                else
                    players[player_count].name[0] = '\0';

                char *hand_tok = strtok_r(NULL, ";", &saveptr2);
                if (hand_tok)
                    strcpy(players[player_count].hand_str, hand_tok);
                else
                    players[player_count].hand_str[0] = '\0';

                char *score_tok = strtok_r(NULL, ";", &saveptr2);
                if (score_tok)
                    players[player_count].score = atoi(score_tok);
                else
                    players[player_count].score = 0;

                char *wallet_tok = strtok_r(NULL, ";", &saveptr2);
                if (wallet_tok)
                    players[player_count].wallet = atoi(wallet_tok);
                else
                    players[player_count].wallet = 0;

                char *color_tok = strtok_r(NULL, ";", &saveptr2);
                if (color_tok)
                    strcpy(players[player_count].color, color_tok);
                else
                    players[player_count].color[0] = '\0';

                player_count++;
            }
            entity = strtok_r(NULL, "|", &saveptr1);
        }
        display_game_table();
    }
    else if (strncmp(server_message, "PROMPT;", 7) == 0)
    {
        char message_copy[BUFFER_SIZE];
        strcpy(message_copy, server_message);
        char *prompt_data = message_copy + 7;
        char *saveptr;
        strtok_r(prompt_data, ";", &saveptr); // Skip the type
        char *text = strtok_r(NULL, ";", &saveptr);

        // Format the prompt nicely
        printf("\033[1;35m┌─ YOUR TURN ────────────────────────────────────────────────────────┐\033[0m\n");
        printf("\033[1;35m│\033[0m  %s ", text);
        fgets(temp_buffer, sizeof(temp_buffer), stdin);
        temp_buffer[strcspn(temp_buffer, "\n")] = '\0'; // Remove newline
        printf("\033[1;35m└────────────────────────────────────────────────────────────────────┘\033[0m\n");
        send(clientSocket, temp_buffer, strlen(temp_buffer), 0);
    }
    else if (strncmp(server_message, "RESULT;", 7) == 0)
    {
        // Format result message nicely
        const char *result_text = server_message + 7;
        printf("\n\033[1;37m╔════════════════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[1;37m║  %s\n", result_text);
        printf("\033[1;37m╚════════════════════════════════════════════════════════════════════╝\033[0m\n\n");
    }
    else if (strncmp(server_message, "NEW_ROUND", 9) == 0)
    {
        // Add a visual separator for new rounds
        printf("\n\033[1;33m═══════════════════════════════ NEW ROUND ═══════════════════════════════\033[0m\n");
        usleep(500000); // 0.5 second pause for smooth transition
    }
    else if (strstr(server_message, "Enter your bet:") || strstr(server_message, "Invalid bet."))
    {
        printf("\033[1;32m▶ \033[0m %s", server_message);
        fgets(temp_buffer, sizeof(temp_buffer), stdin);
        temp_buffer[strcspn(temp_buffer, "\n")] = '\0';
        send(clientSocket, temp_buffer, strlen(temp_buffer), 0);
    }
    else if (strcmp(server_message, "BET_OK") == 0)
    {
        // Visual confirmation
        printf("\033[1;32m✓ Bet placed!\033[0m\n");
        usleep(300000); // 0.3 second pause
    }
    else if (strstr(server_message, "╔════════") ||
             strstr(server_message, "╚════════") ||
             strstr(server_message, "OUT OF CHIPS") ||
             strstr(server_message, "Security is escorting") ||
             strstr(server_message, "Better luck") ||
             strstr(server_message, "escorted out") ||
             strstr(server_message, "Waiting for") ||
             strstr(server_message, "All players are broke") ||
             strstr(server_message, "GAME OVER") ||
             strstr(server_message, "⏳") ||
             strstr(server_message, "🚪") ||
             strstr(server_message, "║"))
    {
        // Special handling for formatted messages - just print without "Server:" prefix
        printf("%s\n", server_message);
    }
    else
    {
        // Fallback for any other messages
        printf("\n\nServer:\n%s", server_message);
    }
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int bytesRead;

    // Seed the random number generator
    srand(time(NULL));

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        error_exit("Socket creation failed");
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        error_exit("Connection to server failed");
    }

    printf("Connected to server.\n");
    printBanner();

    // Receive game mode prompt from server
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0'; // Null-terminate the string
        printf("\nServer:\n%s", buffer);

        // Send game mode selection to server
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        int game_mode_choice = atoi(buffer);
        send(clientSocket, buffer, strlen(buffer), 0);

        // If PvC mode (1), expect difficulty prompt
        if (game_mode_choice == 1)
        {
            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                printf("\nServer:\n%s", buffer);

                // Send difficulty selection
                fgets(buffer, BUFFER_SIZE, stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                send(clientSocket, buffer, strlen(buffer), 0);
            }
            else
            {
                error_exit("Failed to receive difficulty prompt from server");
            }
        }
    }
    else
    {
        error_exit("Failed to receive game mode prompt from server");
    }

    // Receive player name prompt from server
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0'; // Null-terminate the string
        printf("\nServer:\n%s", buffer);

        // Send player name to server
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        send(clientSocket, buffer, strlen(buffer), 0);
    }
    else
    {
        error_exit("Failed to receive player name prompt from server");
    }

    // Game loop
    while (1)
    {
        // Receive messages from server, handling multiple messages in one buffer
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            char *saveptr;
            char *line = strtok_r(buffer, "\n", &saveptr);
            while (line != NULL)
            {
                parse_and_update_state(line, clientSocket);
                line = strtok_r(NULL, "\n", &saveptr);
            }
        }
        else if (bytesRead == 0)
        {
            printf("Connection closed by server.\n");
            break; // Exit loop if connection closed
        }
        else
        {
            error_exit("recv failed");
        }
    }

    // Cleanup
    close(clientSocket);

    printBanner(); // Show the banner one last time at the end.
    printf("%s\nGame Over.\n\n%s", getColor(0), "\033[0m");

    // Wait for user input before exiting
    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}