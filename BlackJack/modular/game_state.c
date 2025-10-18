#include "game_state.h"
#include "player.h"
#include "stack.h"
#include "utils.h"
#include "server_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void deal_initial_cards(Player players[], int player_count, Player *dealer, Stack *cardStack)
{
    for (int i = 0; i < player_count; i++)
    {
        players[i].hand_size = 0; // Clear player hands
        for (int j = 0; j < 2; j++)
        {
            players[i].hand[players[i].hand_size++] = pop(cardStack);
        }
    }
    // Deal two cards to the dealer
    dealer->hand_size = 0; // Clear dealer hand
    for (int j = 0; j < 2; j++)
    {
        dealer->hand[dealer->hand_size++] = pop(cardStack);
    }

    // Calculate players' initial scores after dealing cards
    for (int i = 0; i < player_count; i++)
    {
        calculate_score(&players[i], players, player_count, dealer);
    }

    // Calculate dealer's score last
    calculate_score(dealer, players, player_count, dealer);
}

// Helper function to build a structured game state string
int build_game_state_string(char *buffer, size_t buffer_size, Player players[], int player_count, Player *dealer, bool hide_dealer_card)
{
    int offset = 0;
    char hand_buffer[512];

    // Build dealer hand string
    hand_buffer[0] = '\0'; // Clear the hand buffer before use
    int hand_offset = 0;
    for (int i = 0; i < dealer->hand_size; i++)
    {
        if (hide_dealer_card && i == 1)
        {
            hand_offset += snprintf(hand_buffer + hand_offset, sizeof(hand_buffer) - hand_offset, "[Hidden]");
        }
        else
        {
            // Get card string and immediately copy it to avoid static buffer issues
            const char *card_str = card_to_string(dealer->hand[i]);
            char temp_card[64];
            strncpy(temp_card, card_str, sizeof(temp_card) - 1);
            temp_card[sizeof(temp_card) - 1] = '\0';
            hand_offset += snprintf(hand_buffer + hand_offset, sizeof(hand_buffer) - hand_offset, "%s", temp_card);
        }
        if (i < dealer->hand_size - 1)
        {
            hand_offset += snprintf(hand_buffer + hand_offset, sizeof(hand_buffer) - hand_offset, ", ");
        }
    }
    offset += snprintf(buffer + offset, buffer_size - offset, "DEALER;%s;%d|", hand_buffer, hide_dealer_card ? 0 : dealer->score);

    // Build each player's hand string
    for (int i = 0; i < player_count; i++)
    {
        // Skip players who have been kicked out (wallet <= 0 or inactive)
        if (players[i].wallet <= 0)
        {
            continue;
        }

        hand_buffer[0] = '\0'; // Clear the hand buffer for the next player
        hand_offset = 0;
        for (int j = 0; j < players[i].hand_size; j++)
        {
            // Get card string and immediately copy it to avoid static buffer issues
            const char *card_str = card_to_string(players[i].hand[j]);
            char temp_card[64];
            strncpy(temp_card, card_str, sizeof(temp_card) - 1);
            temp_card[sizeof(temp_card) - 1] = '\0';
            hand_offset += snprintf(hand_buffer + hand_offset, sizeof(hand_buffer) - hand_offset, "%s", temp_card);
            if (j < players[i].hand_size - 1)
            {
                hand_offset += snprintf(hand_buffer + hand_offset, sizeof(hand_buffer) - hand_offset, ", ");
            }
        }
        // Ensure color field is not empty - use a placeholder if needed
        const char *color_code = (players[i].color[0] != '\0') ? players[i].color : "\033[37m";
        offset += snprintf(buffer + offset, buffer_size - offset, "PLAYER;%s;%s;%d;%d;%s|", players[i].name, hand_buffer, players[i].score, players[i].wallet, color_code);
    }
    return offset;
}

void send_game_state(Player players[], int player_count, Player *dealer)
{
    char buffer[BUFFER_SIZE];
    build_game_state_string(buffer, sizeof(buffer), players, player_count, dealer, false);

    // Send the game state to all players
    for (int i = 0; i < player_count; i++)
    {
        send(players[i].socket, buffer, strlen(buffer), 0);
    }
}

void prompt_player_action(Player players[], int player_count, Player *player, Player *dealer, Stack *cardStack)
{
    char buffer[BUFFER_SIZE];
    int bytesRead;

    // Notify other players to wait
    char waiting_msg[256];
    snprintf(waiting_msg, sizeof(waiting_msg),
        "\n\033[1;90m⏳ Waiting for %s to make their move...\033[0m\n", player->name);

    for (int i = 0; i < player_count; i++)
    {
        if (&players[i] != player && players[i].is_active)
        {
            send(players[i].socket, waiting_msg, strlen(waiting_msg), 0);
        }
    }

    while (player->is_active)
    {
        // Combine game state and prompt into a single message
        int offset = 0;
        calculate_score(player, players, player_count, dealer); // Calculate score before prompting

        if (player->score > 21)
        {
            player->is_active = 0; // Player is busted, break the loop
            break;
        }

        // Send structured state and prompt
        offset = snprintf(buffer, sizeof(buffer), "STATE;");
        offset += build_game_state_string(buffer + offset, sizeof(buffer) - offset, players, player_count, dealer, true);
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\nPROMPT;hit_stand;Your turn: hit or stand?");

        // Send the combined message to the player
        send(player->socket, buffer, strlen(buffer), 0);

        // Receive action from player
        bytesRead = recv(player->socket, buffer, BUFFER_SIZE, 0);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            // Remove trailing newline/whitespace
            char *newline = strchr(buffer, '\n');
            if (newline) *newline = '\0';
            char *carriage = strchr(buffer, '\r');
            if (carriage) *carriage = '\0';

            printf("Received from %s: %s\n", player->name, buffer);

            if (strcmp(buffer, "hit") == 0)
            {
                // Deal a new card to the player
                if (isEmpty(cardStack))
                {
                    fillStack(cardStack);
                }
                player->hand[player->hand_size++] = pop(cardStack);
                calculate_score(player, players, player_count, dealer);
                print_debug_info(dealer, players, player_count);

                // Re-display the server gamestate after a hit
                display_server_gamestate(player, cardStack);
            }
            else if (strcmp(buffer, "stand") == 0)
            {
                player->is_active = 0;
                print_debug_info(dealer, players, player_count);
            }
            else
            {
                send(player->socket, "Invalid action. Please type 'hit' or 'stand'.\n\n", 47, 0);
            }
        }
        else if (bytesRead <= 0)
        {
            // Handle player disconnect
            printf("Player %s disconnected. recv failed: %s\n", player->name, strerror(errno));
            player->is_active = 0;
        }
    }

    // If the player busted, inform them.
    if (player->score > 21)
    {
        char busted_buffer[BUFFER_SIZE];
        int offset = snprintf(busted_buffer, sizeof(busted_buffer), "STATE;");
        offset += build_game_state_string(busted_buffer + offset, sizeof(busted_buffer) - offset, players, player_count, dealer, false);
        offset += snprintf(busted_buffer + offset, sizeof(busted_buffer) - offset, "\nRESULT;You are busted!");
        send(player->socket, busted_buffer, strlen(busted_buffer), 0);
    }
}

void dealer_turn(Player *dealer, Stack *cardStack, Player players[], int player_count)
{
    printf("\n\033[1;33m--- Dealer's Turn ---\033[0m\n");
    display_server_gamestate(dealer, cardStack); // Show initial state

    while (dealer->score < 17)
    {
        printf("\033[1;33mDealer hits.\033[0m\n");
        usleep(1000000); // Pause for 1 second to make it observable
        if (isEmpty(cardStack))
        {
            fillStack(cardStack);
        }
        dealer->hand[dealer->hand_size++] = pop(cardStack);
        calculate_score(dealer, players, player_count, dealer);
        display_server_gamestate(dealer, cardStack);
    }

    if (dealer->score <= 21)
    {
        printf("\033[1;33mDealer stands.\033[0m\n");
    }
    else
    {
        printf("\033[1;31mDealer busts!\033[0m\n");
    }
    printf("\033[1;33m---------------------\033[0m\n");
}

void dealer_turn_smart(Player *dealer, Stack *cardStack, Player players[], int player_count)
{
    int highest_player_score = 0;
    int active_players = 0;

    for (int i = 0; i < player_count; i++)
    {
        if (players[i].score <= 21)
        { // Only count non-busted players
            active_players++;
            if (players[i].score > highest_player_score)
            {
                highest_player_score = players[i].score;
            }
        }
    }

    printf("\n\033[1;33m--- Dealer's Turn (Smart AI) ---\033[0m\n");
    display_server_gamestate(dealer, cardStack); // Show initial state

    // If all players busted, dealer doesn't need to play
    if (active_players == 0)
    {
        printf("\033[1;33mAll players busted. Dealer stands.\033[0m\n");
        printf("\033[1;33m----------------------------------\033[0m\n");
        return;
    }

    // Smart strategy: dealer tries to beat the highest player
    while (dealer->score < 17)
    {
        printf("\033[1;33mDealer hits (score < 17).\033[0m\n");
        usleep(1000000);
        if (isEmpty(cardStack))
        {
            fillStack(cardStack);
        }
        dealer->hand[dealer->hand_size++] = pop(cardStack);
        calculate_score(dealer, players, player_count, dealer);
        display_server_gamestate(dealer, cardStack);
    }

    // Additional smart logic: if dealer is between 17-21 but still losing,
    // consider the risk of hitting based on probability
    while (dealer->score >= 17 && dealer->score < highest_player_score && dealer->score <= 21)
    {
        // If dealer has soft 17 (Ace counted as 11), always hit
        int has_soft_ace = 0;
        int ace_count = 0;

        for (int i = 0; i < dealer->hand_size; i++)
        {
            if (dealer->hand[i] % 13 == 0)
            { // Ace
                ace_count++;
            }
        }

        // Check if we have a soft hand (Ace counted as 11)
        if (ace_count > 0 && dealer->score + 10 <= 21)
        {
            has_soft_ace = 1;
        }

        // Decision logic
        if (dealer->score == 17 && has_soft_ace)
        {
            printf("\033[1;33mDealer hits on soft 17.\033[0m\n");
            usleep(1000000);
            if (isEmpty(cardStack))
            {
                fillStack(cardStack);
            }
            dealer->hand[dealer->hand_size++] = pop(cardStack);
            calculate_score(dealer, players, player_count, dealer);
            display_server_gamestate(dealer, cardStack);
        }
        else if (dealer->score <= 18 && highest_player_score >= 19)
        {
            printf("\033[1;33mDealer takes a risk to beat player score of %d.\033[0m\n", highest_player_score);
            usleep(1000000);
            if (isEmpty(cardStack))
            {
                fillStack(cardStack);
            }
            dealer->hand[dealer->hand_size++] = pop(cardStack);
            calculate_score(dealer, players, player_count, dealer);
            display_server_gamestate(dealer, cardStack);
        }
        else
        {
            break;
        }
    }

    if (dealer->score <= 21)
    {
        printf("\033[1;33mDealer stands.\033[0m\n");
    }
    else
    {
        printf("\033[1;31mDealer busts!\033[0m\n");
    }
    printf("\033[1;33m----------------------------------\033[0m\n");
}

void determine_winners(Player players[], int player_count, Player *dealer)
{
    // Determine and send the result to each player
    for (int i = 0; i < player_count; i++)
    {
        char final_buffer[BUFFER_SIZE];
        int offset = 0;

        // Build the final game state string
        offset = snprintf(final_buffer, sizeof(final_buffer), "STATE;");
        offset += build_game_state_string(final_buffer + offset, sizeof(final_buffer) - offset, players, player_count, dealer, false);

        char result_msg[100];
        // Append the result message to the same buffer
        if (players[i].score > 21)
        {
            players[i].wallet -= players[i].current_bet;
            snprintf(result_msg, sizeof(result_msg), "\033[31mYou lost \033[32m$%d\033[31m!\033[0m", players[i].current_bet);
        }
        else if (dealer->score > 21 || players[i].score > dealer->score)
        {
            // Blackjack (21 on first 2 cards) pays 3:2
            if (players[i].score == 21 && players[i].hand_size == 2)
            {
                players[i].wallet += (players[i].current_bet * 3) / 2;
                snprintf(result_msg, sizeof(result_msg), "\033[36mBlackjack! You won \033[32m$%d\033[36m!\033[0m", (players[i].current_bet * 3) / 2);
            }
            else
            {
                players[i].wallet += players[i].current_bet;
                snprintf(result_msg, sizeof(result_msg), "\033[36mYou won \033[32m$%d\033[36m!\033[0m", players[i].current_bet);
            }
        }
        else if (players[i].score == dealer->score)
        {
            snprintf(result_msg, sizeof(result_msg), "Push! Your bet of \033[32m$%d\033[0m is returned.", players[i].current_bet);
        }
        else
        {
            players[i].wallet -= players[i].current_bet;
            snprintf(result_msg, sizeof(result_msg), "\033[31mYou lost \033[32m$%d\033[31m!\033[0m", players[i].current_bet);
        }

        offset += snprintf(final_buffer + offset, sizeof(final_buffer) - offset, "\nRESULT;%s", result_msg);

        // Only send "Play again?" prompt if player still has money
        if (players[i].wallet > 0)
        {
            offset += snprintf(final_buffer + offset, sizeof(final_buffer) - offset, "\nPROMPT;play_again;Play again? (yes/no):");
        }

        // Send the single, consolidated message
        send(players[i].socket, final_buffer, strlen(final_buffer), 0);
    }
}