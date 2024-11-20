#include "game_state.h"
#include "player.h"
#include "stack.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

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
        calculate_score(player, players, player_count, dealer); // Calculate score before prompting for action
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