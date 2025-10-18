#ifndef UTILS_H
#define UTILS_H

#define BUFFER_SIZE 4096 // Increased buffer size

#include "player.h" // Include the header file where Player is defined
#include "stack.h"

extern const char *DEALER_STRING;
extern int debugCounter;

const char *card_to_string(int card);
const char *getRandomColor();
void printBanner();
void error_exit(const char *message);
const char *getColor(int choice);
void print_debug_info(Player *dealer, Player players[], int player_count);
int visible_strlen(const char *str);

#endif // UTILS_H