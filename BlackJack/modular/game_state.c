#include "game_state.h"
#include "player.h"
#include "stack.h"
#include "utils.h"
#include "server_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

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

    // Calculate players' initial scores after dealing cards
    for (int i = 0; i < player_count; i++) {
        calculate_score(&players[i], players, player_count, dealer);
    }

    // Calculate dealer's score last
    calculate_score(dealer, players, player_count, dealer);
}

// Helper function to build the game state string
int build_game_state_string(char *buffer, size_t buffer_size, Player players[], int player_count, Player *dealer) {
    int offset = 0;

    // Add dealer's cards to the buffer
    offset += snprintf(buffer + offset, buffer_size - offset, " %s", DEALER_STRING);
    for (int i = 0; i < dealer->hand_size; i++) {
        offset += snprintf(buffer + offset, buffer_size - offset, " | %s", card_to_string(dealer->hand[i]));
    }
    offset += snprintf(buffer + offset, buffer_size - offset, "\n");

    // Add each player's cards to the buffer
    for (int i = 0; i < player_count; i++) {
        offset += snprintf(buffer + offset, buffer_size - offset, " %s%s:\t\033[0m", players[i].color, players[i].name);
        for (int j = 0; j < players[i].hand_size; j++) {
            offset += snprintf(buffer + offset, buffer_size - offset, " | %s", card_to_string(players[i].hand[j]));
        }
        offset += snprintf(buffer + offset, buffer_size - offset, "\n");
    }
    return offset;
}

void send_game_state(Player players[], int player_count, Player *dealer) {
    char buffer[BUFFER_SIZE];
    build_game_state_string(buffer, sizeof(buffer), players, player_count, dealer);

    // Send the game state to all players
    for (int i = 0; i < player_count; i++) {
        send(players[i].socket, buffer, strlen(buffer), 0);
    }
}

void prompt_player_action(Player players[], int player_count, Player *player, Player *dealer, Stack *cardStack) {
    char buffer[BUFFER_SIZE];
    int bytesRead;

    while (player->is_active) {
        // Combine game state and prompt into a single message
        int offset = 0;
        calculate_score(player, players, player_count, dealer); // Calculate score before prompting

        if (player->score > 21) {
            player->is_active = 0; // Player is busted, break the loop
            break;
        }

        offset = build_game_state_string(buffer, sizeof(buffer), players, player_count, dealer);
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
                calculate_score(player, players, player_count, dealer);
                print_debug_info(dealer, players, player_count);

                // Re-display the server gamestate after a hit
                display_server_gamestate(player, cardStack);

            } else if (strcmp(buffer, "stand") == 0) {
                player->is_active = 0;
                print_debug_info(dealer, players, player_count);
            } else {
                send(player->socket, "Invalid action. Please type 'hit' or 'stand'.\n", 45, 0);
            }
        } else if (bytesRead <= 0) {
            // Handle player disconnect
            printf("Player %s disconnected. recv failed: %s\n", player->name, strerror(errno));
            player->is_active = 0;
        }
    }

    // If the player busted, inform them.
    if (player->score > 21) {
        char busted_buffer[BUFFER_SIZE];
        int offset = build_game_state_string(busted_buffer, sizeof(busted_buffer), players, player_count, dealer);
        offset += snprintf(busted_buffer + offset, sizeof(busted_buffer) - offset, "You are busted!\n");
        send(player->socket, busted_buffer, strlen(busted_buffer), 0);
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
    // Determine and send the result to each player
    for (int i = 0; i < player_count; i++) {
        char final_buffer[BUFFER_SIZE];
        int offset = 0;

        // Build the final game state string first
        offset = build_game_state_string(final_buffer, sizeof(final_buffer), players, player_count, dealer);

        // Append the result message to the same buffer
        if (players[i].score > 21) {
            offset += snprintf(final_buffer + offset, sizeof(final_buffer) - offset, "\033[31mYou lost!\033[0m\n"); // Red color
        } else if (dealer->score > 21 || players[i].score > dealer->score) {
            offset += snprintf(final_buffer + offset, sizeof(final_buffer) - offset, "\033[36mYou won!\033[0m\n"); // Cyan color
        } else if (players[i].score == dealer->score) {
            offset += snprintf(final_buffer + offset, sizeof(final_buffer) - offset, "It's a tie!\n");
        } else {
            offset += snprintf(final_buffer + offset, sizeof(final_buffer) - offset, "\033[31mYou lost!\033[0m\n"); // Red color
        }

        // Append the "Play again?" prompt to the same buffer
        offset += snprintf(final_buffer + offset, sizeof(final_buffer) - offset, "\nPlay again? (yes/no): ");

        // Send the single, consolidated message
        send(players[i].socket, final_buffer, strlen(final_buffer), 0);
    }
}