#ifndef PLAYER_H
#define PLAYER_H

#include <winsock2.h>

#define MAX_PLAYERS 4

typedef struct {
    SOCKET socket;
    char name[50];
    int hand[10];
    int hand_size;
    int score;
    int is_active;
    char color[10];
} Player;

void display_player_cards(Player *player);
void calculate_score(Player *player, Player players[], int player_count, Player *dealer);
void reset_player_states(Player players[], int player_count);

#endif // PLAYER_H