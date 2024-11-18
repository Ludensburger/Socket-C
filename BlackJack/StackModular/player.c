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

void reset_player_states(Player players[], int player_count) {
    for (int i = 0; i < player_count; i++) {
        players[i].hand_size = 0;
        players[i].score = 0;
        players[i].is_active = 1;
        strcpy(players[i].color, getRandomColor()); // Reassign random color to each player
    }
}