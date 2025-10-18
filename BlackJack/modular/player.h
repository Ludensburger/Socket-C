#ifndef PLAYER_H
#define PLAYER_H

#define MAX_PLAYERS 4

typedef struct {
    int socket;
    char name[50];
    int hand[10];
    int hand_size;
    int score;
    int wallet;
    int current_bet;
    int is_active;
    char color[20];  // Increased from 10 to 20 to handle longer ANSI codes
} Player;

void display_player_cards(Player *player);
void calculate_score(Player *player, Player players[], int player_count, Player *dealer);
void reset_player_states(Player players[], int player_count);

#endif // PLAYER_H