#include "player.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void display_player_cards(Player *player) {
    printf("Player %s's cards: ", player->name);
    for (int i = 0; i < player->hand_size; i++) {
        printf("%d ", player->hand[i]);
    }
    printf("\n");
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

void reset_player_states(Player players[], int player_count) {
    for (int i = 0; i < player_count; i++) {
        players[i].hand_size = 0;
        players[i].score = 0;
        players[i].is_active = 1;
        strcpy(players[i].color, getRandomColor()); // Reassign random color to each player
    }
}