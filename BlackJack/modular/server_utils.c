#include "server_utils.h"
#include "utils.h" // For card_to_string, printStack, etc.
#include <stdio.h>

void display_server_gamestate(Player *player, Stack *cardStack) {
    // Print the current state of the deck
    printStack(cardStack, 1); // player_count doesn't matter here

    if (isEmpty(cardStack)) {
        printf("\n---\nDeck is empty. Will be reshuffled on next draw.\n---\n");
        return;
    }

    // --- Enhanced Server-Side Turn Information ---
    int next_card = cardStack->cards[cardStack->top];
    int potential_score = player->score;
    int card_value = next_card % 13;
    if (card_value >= 10) potential_score += 10;
    else if (card_value == 0) potential_score += 11; // Ace
    else potential_score += card_value + 1;
    // Simplified Ace adjustment for prediction
    if (potential_score > 21 && player->score < 11) {
        potential_score -= 10;
    }

    printf("\n---\nIt's %s's turn. Next card in deck: %s\n", player->name, card_to_string(next_card));
    display_player_cards(player);
    printf("If player hits, potential new score: %d\n---\n", potential_score);
}