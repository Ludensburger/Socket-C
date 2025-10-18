#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "player.h"
#include "stack.h"
#include <stddef.h> // For size_t

void deal_initial_cards(Player players[], int player_count, Player *dealer, Stack *cardStack);

int build_game_state_string(char *buffer, size_t buffer_size, Player players[], int player_count, Player *dealer);

void send_game_state(Player players[], int player_count, Player *dealer);

void prompt_player_action(Player players[], int player_count, Player *player, Player *dealer, Stack *cardStack);

void dealer_turn(Player *dealer, Stack *cardStack, Player players[], int player_count);

void determine_winners(Player players[], int player_count, Player *dealer);

#endif // GAME_STATE_H