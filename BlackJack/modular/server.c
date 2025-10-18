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

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    Player players[MAX_PLAYERS];
    int player_count = 0;
    Stack cardStack;
    int game_mode = 0;
    int difficulty = 1; // 1=Easy (current), 2=Medium, 3=Hard (smart AI)

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Allow the socket to be reused immediately after it's closed
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        close(serverSocket);
        return 1;
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_PLAYERS) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        return 1;
    }

    printf("Waiting for connections...\n");

    // Accept the first player connection
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientSocket < 0)
    {
        perror("Accept failed");
        close(serverSocket);
        return 1;
    }

    // When accepting the first player connection
    players[player_count].socket = clientSocket;
    players[player_count].hand_size = 0;
    players[player_count].score = 0;
    players[player_count].is_active = 1;
    players[player_count].wallet = 100; // Starting wallet

    // Assign a random color to the player
    const char *playerColor = getRandomColor();
    strcpy(players[player_count].color, playerColor); // Assign random color to Player 1
    player_count++;
    printf("Player 1 connected.\n");

    // Prompt the first player to choose the game mode
    const char *game_mode_prompt =
        "\n\033[1;36m╔════════════════════════════════════╗\033[0m\n"
        "\033[1;36m║      \033[1;33m Select Game Mode\033[1;36m             ║\033[0m\n"
        "\033[1;36m╠════════════════════════════════════╣\033[0m\n"
        "\033[1;36m║  \033[1;32m1\033[0m - Player vs Computer (PvC)     \033[1;36m ║\033[0m\n"
        "\033[1;36m║  \033[1;33m2\033[0m - Player vs Player (1v1)       \033[1;36m ║\033[0m\n"
        "\033[1;36m║  \033[1;34m3\033[0m - Player vs Player (1v2)       \033[1;36m ║\033[0m\n"
        "\033[1;36m║  \033[1;35m4\033[0m - Player vs Player (1v3)       \033[1;36m ║\033[0m\n"
        "\033[1;36m║  \033[1;36m5\033[0m - Player vs Player (1v4)       \033[1;36m ║\033[0m\n"
        "\033[1;36m╚════════════════════════════════════╝\033[0m\n"
        "Enter your choice (1-5): ";
    send(clientSocket, game_mode_prompt, strlen(game_mode_prompt), 0);

    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0'; // Null-terminate the string
        game_mode = atoi(buffer);
        printf("Game mode selected: %d\n", game_mode);

        // Validate the game mode
        if (game_mode < 1 || game_mode > 5)
        {
            printf("Invalid game mode. Exiting...\n");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        // If PvC mode, ask for difficulty
        if (game_mode == 1)
        {
            const char *difficulty_prompt =
                "\n\033[1;36m╔════════════════════════════════════╗\033[0m\n"
                "\033[1;36m║      \033[1;33m Select Difficulty\033[1;36m            ║\033[0m\n"
                "\033[1;36m╠════════════════════════════════════╣\033[0m\n"
                "\033[1;36m║  \033[1;32m1\033[0m - Easy   (Dealer hits <17)     \033[1;36m ║\033[0m\n"
                "\033[1;36m║  \033[1;33m2\033[0m - Medium (Dealer plays smart)  \033[1;36m ║\033[0m\n"
                "\033[1;36m║  \033[1;31m3\033[0m - Hard   (Coming soon...)      \033[1;36m ║\033[0m\n"
                "\033[1;36m╚════════════════════════════════════╝\033[0m\n"
                "Enter your choice (1-2): ";
            send(clientSocket, difficulty_prompt, strlen(difficulty_prompt), 0);

            bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                difficulty = atoi(buffer);
                if (difficulty < 1 || difficulty > 2)
                {
                    difficulty = 1; // Default to easy
                }
                printf("Difficulty selected: %d\n", difficulty);
            }
        }

        // Enter player name for Player 1
        send(clientSocket, "Enter your name: \n", 19, 0);
        bytesRead = recv(clientSocket, players[0].name, sizeof(players[0].name) - 1, 0);
        if (bytesRead > 0)
        {
            players[0].name[bytesRead] = '\0'; // Null-terminate the string
            // Remove trailing newline/whitespace
            char *newline = strchr(players[0].name, '\n');
            if (newline) *newline = '\0';
            char *carriage = strchr(players[0].name, '\r');
            if (carriage) *carriage = '\0';
            printf("Player 1 name: %s\n", players[0].name);
        }
        else
        {
            perror("recv failed for player 1 name");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        int required_players = game_mode == 1 ? 1 : game_mode;

        // Accept additional player connections if needed
        for (int i = 1; i < required_players; i++)
        {
            clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
            if (clientSocket < 0)
            {
                perror("Accept failed for additional player");
                close(serverSocket);
                return 1;
            }

            players[player_count].socket = clientSocket;
            players[player_count].hand_size = 0;
            players[player_count].score = 0;
            players[player_count].is_active = 1;
            players[player_count].wallet = 100; // Starting wallet

            const char *otherPlayerColor = getRandomColor();

            while (playerColor == otherPlayerColor)
            {
                otherPlayerColor = getRandomColor();
            }

            strcpy(players[player_count].color, otherPlayerColor); // Assign random color to additional players
            player_count++;
            printf("Player %d connected.\n", i + 1);

            // Enter player name for additional players
            send(clientSocket, "Enter your name: \n", 19, 0);
            bytesRead = recv(clientSocket, players[player_count - 1].name, sizeof(players[player_count - 1].name) - 1, 0);
            if (bytesRead > 0)
            {
                players[player_count - 1].name[bytesRead] = '\0'; // Null-terminate the string
                // Remove trailing newline/whitespace
                char *newline = strchr(players[player_count - 1].name, '\n');
                if (newline) *newline = '\0';
                char *carriage = strchr(players[player_count - 1].name, '\r');
                if (carriage) *carriage = '\0';
                printf("Player %d name: %s\n", i + 1, players[player_count - 1].name);
            }
            else
            {
                perror("recv failed for additional player name");
                close(clientSocket);
                close(serverSocket);
                return 1;
            }
        }
    }
    else
    {
        perror("recv failed for game mode");
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    int play_again = 1;
    while (play_again)
    {
        // --- Betting Phase ---
        // First check how many players still have money
        int solvent_players = 0;
        for (int i = 0; i < player_count; i++)
        {
            if (players[i].wallet > 0)
            {
                solvent_players++;
            }
        }

        // If no players have money, end the game
        if (solvent_players == 0)
        {
            const char *all_broke =
                "\n\033[1;31m╔════════════════════════════════════════════╗\033[0m\n"
                "\033[1;31m║                                            ║\033[0m\n"
                "\033[1;31m║        ALL PLAYERS ARE OUT OF CHIPS        ║\033[0m\n"
                "\033[1;31m║                                            ║\033[0m\n"
                "\033[1;33m║         The casino thanks you all.         ║\033[0m\n"
                "\033[1;31m║                                            ║\033[0m\n"
                "\033[1;31m╚════════════════════════════════════════════╝\033[0m\n\n"
                "\033[90m      The house always wins in the end.\033[0m\n\n"
                "\033[1;33m                  GAME OVER\033[0m\n"
                "\n";
            for (int i = 0; i < player_count; i++)
            {
                send(players[i].socket, all_broke, strlen(all_broke), 0);
            }
            play_again = 0;
            break;
        }

        for (int i = 0; i < player_count; i++)
        {
            // Check if player is broke
            if (players[i].wallet <= 0)
            {
                // Create broke message with thick border like game mode selection
                char broke_message[1024];
                snprintf(broke_message, sizeof(broke_message),
                    "\n\033[1;31m╔════════════════════════════════════════════╗\033[0m\n"
                    "\033[1;31m║                                            ║\033[0m\n"
                    "\033[1;31m║     UNFORTUNATELY, YOU'RE OUT OF CHIPS     ║\033[0m\n"
                    "\033[1;31m║                                            ║\033[0m\n"
                    "\033[1;33m║      Security is escorting you out.        ║\033[0m\n"
                    "\033[1;31m║                                            ║\033[0m\n"
                    "\033[1;31m╚════════════════════════════════════════════╝\033[0m\n\n"
                    "\033[90m        Better luck next time, high roller.\033[0m\n\n"
                    "\033[1;33m                  GAME OVER\033[0m\n"
                    "\n");

                // Notify the broke player
                send(players[i].socket, broke_message, strlen(broke_message), 0);

                // Notify all other players
                char other_msg[256];
                snprintf(other_msg, sizeof(other_msg),
                    "\n\033[1;33m🚪 %s has been escorted out (ran out of chips)\033[0m\n", players[i].name);

                for (int j = 0; j < player_count; j++)
                {
                    if (j != i && players[j].wallet > 0)
                    {
                        send(players[j].socket, other_msg, strlen(other_msg), 0);
                    }
                }

                players[i].is_active = 0;
                close(players[i].socket);
                continue; // Skip betting for this player
            }

            char prompt_buffer[120];
            snprintf(prompt_buffer, sizeof(prompt_buffer), "You have \033[32m$%d\033[0m. Enter your bet: \n", players[i].wallet);
            send(players[i].socket, prompt_buffer, strlen(prompt_buffer), 0);

            int valid_bet = 0;
            while (!valid_bet)
            {
                bytesRead = recv(players[i].socket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    // Remove trailing newline/whitespace
                    char *newline = strchr(buffer, '\n');
                    if (newline) *newline = '\0';
                    char *carriage = strchr(buffer, '\r');
                    if (carriage) *carriage = '\0';

                    int bet_amount = atoi(buffer);
                    if (bet_amount > 0 && bet_amount <= players[i].wallet)
                    {
                        players[i].current_bet = bet_amount;
                        valid_bet = 1;
                        send(players[i].socket, "BET_OK\n", 7, 0);
                    }
                    else
                    {
                        send(players[i].socket, "Invalid bet. Please enter a valid amount: \n", 43, 0);
                    }
                }
                else
                {
                    // Handle disconnect during betting
                    players[i].is_active = 0;
                    break;
                }
            }
        }

        // If any player is broke, exit the game loop
        if (!play_again)
        {
            break;
        }

        // Initialize and fill the card stack with a new seed
        srand(time(NULL)); // Use the current time as the seed for the random number generator
        resetAndFillStack(&cardStack);

        // Initialize dealer
        Player dealer;
        dealer.hand_size = 0;
        dealer.score = 0;
        dealer.is_active = 1;

        // Reset player states for the new round
        reset_player_states(players, player_count);

        // Send only the "NEW_ROUND" signal to all clients
        for (int i = 0; i < player_count; i++)
        {
            send(players[i].socket, "NEW_ROUND\n", 10, 0);
        }

        // Deal initial cards
        deal_initial_cards(players, player_count, &dealer, &cardStack);

        // Print the deck state *after* initial cards have been dealt for an accurate log
        printStack(&cardStack, player_count);

        // Send initial game state to all clients (with dealer's second card hidden)
        char initial_state[BUFFER_SIZE];
        int offset = snprintf(initial_state, sizeof(initial_state), "STATE;");
        offset += build_game_state_string(initial_state + offset, sizeof(initial_state) - offset, players, player_count, &dealer, true);
        offset += snprintf(initial_state + offset, sizeof(initial_state) - offset, "\n");

        // DEBUG: Print what we're sending
        printf("=== INITIAL STATE DEBUG ===\n");
        printf("Sending state: %s", initial_state);
        for (int i = 0; i < player_count; i++)
        {
            printf("Player %d (%s): hand_size=%d, score=%d, wallet=%d, color='%s' (len=%lu)\n",
                   i, players[i].name, players[i].hand_size, players[i].score, players[i].wallet,
                   players[i].color, strlen(players[i].color));
        }
        printf("Dealer: hand_size=%d, score=%d\n", dealer.hand_size, dealer.score);
        printf("===========================\n");

        for (int i = 0; i < player_count; i++)
        {
            send(players[i].socket, initial_state, strlen(initial_state), 0);
        }

        // Player turns
        for (int i = 0; i < player_count; i++)
        {
            if (players[i].is_active)
            {
                // Display the dynamic server-side game state
                display_server_gamestate(&players[i], &cardStack);
                prompt_player_action(players, player_count, &players[i], &dealer, &cardStack);
            }
        }

        // Dealer's turn - use appropriate AI based on difficulty
        if (game_mode == 1 && difficulty == 2)
        {
            // Medium difficulty: Smart AI
            printf("\n[DEALER AI: Using Smart Strategy]\n");
            dealer_turn_smart(&dealer, &cardStack, players, player_count);
        }
        else
        {
            // Easy difficulty or PvP: Standard casino rules
            dealer_turn(&dealer, &cardStack, players, player_count);
        }
        print_debug_info(&dealer, players, player_count); // Add final debug info after dealer's turn

        // Determine winners
        determine_winners(players, player_count, &dealer);

        // Small delay to let players see their results
        usleep(500000); // 0.5 second

        // Check if any players went broke after this round and notify them
        for (int i = 0; i < player_count; i++)
        {
            if (players[i].wallet <= 0 && players[i].is_active)
            {
                // Create broke message with thick border
                char broke_message[1024];
                snprintf(broke_message, sizeof(broke_message),
                    "\n\033[1;31m╔════════════════════════════════════════════╗\033[0m\n"
                    "\033[1;31m║                                            ║\033[0m\n"
                    "\033[1;31m║     UNFORTUNATELY, YOU'RE OUT OF CHIPS     ║\033[0m\n"
                    "\033[1;31m║                                            ║\033[0m\n"
                    "\033[1;33m║      Security is escorting you out.        ║\033[0m\n"
                    "\033[1;31m║                                            ║\033[0m\n"
                    "\033[1;31m╚════════════════════════════════════════════╝\033[0m\n\n"
                    "\033[90m        Better luck next time, high roller.\033[0m\n\n"
                    "\033[1;33m                  GAME OVER\033[0m\n"
                    "\n");

                // Notify the broke player
                send(players[i].socket, broke_message, strlen(broke_message), 0);

                // Notify all other players
                char other_msg[256];
                snprintf(other_msg, sizeof(other_msg),
                    "\n\033[1;33m🚪 %s has been escorted out (ran out of chips)\033[0m\n", players[i].name);

                for (int j = 0; j < player_count; j++)
                {
                    if (j != i && players[j].wallet > 0)
                    {
                        send(players[j].socket, other_msg, strlen(other_msg), 0);
                    }
                }

                players[i].is_active = 0;
                close(players[i].socket);
            }
        }

        // Ask all players if they want to play again
        int players_want_continue = 0;
        for (int i = 0; i < player_count; i++)
        {
            if (players[i].wallet > 0) // Only ask players who still have money
            {
                bytesRead = recv(players[i].socket, buffer, BUFFER_SIZE, 0);
                if (bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    // Remove trailing newline/whitespace
                    char *newline = strchr(buffer, '\n');
                    if (newline) *newline = '\0';
                    char *carriage = strchr(buffer, '\r');
                    if (carriage) *carriage = '\0';

                    if (strcmp(buffer, "yes") == 0)
                    {
                        players_want_continue++;
                    }
                }
                else
                {
                    // Player disconnected
                    players[i].is_active = 0;
                }
            }
        }

        // Continue only if at least one player wants to continue
        if (players_want_continue == 0)
        {
            play_again = 0;
        }
    }

    // Cleanup
    for (int i = 0; i < player_count; i++)
    {
        close(players[i].socket);
    }
    close(serverSocket);

    // Clean the stack at the end of the game
    cleanStack(&cardStack);

    return 0;
}