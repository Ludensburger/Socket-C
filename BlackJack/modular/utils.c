#include "utils.h"
#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *DEALER_STRING = "\033[1;33mDealer:\033[0m"; // Yellow color
int debugCounter = 1;

const char *card_to_string(int card) {
    static char buffer[64];
    const char *values[] = {"Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
    const char *suits[] = {"Spades", "Hearts", "Diamonds", "Clubs"};
    // IMPORTANT: Use only simple color codes WITHOUT semicolons to avoid protocol parsing issues
    const char *colors[] = {"\033[90m", "\033[31m", "\033[36m", "\033[32m"}; // Gray, Red, Cyan, Green

    int value_index = card % 13;
    int suit_index = card / 13;

    snprintf(buffer, sizeof(buffer), "%s%s of %s\033[0m", colors[suit_index], values[value_index], suits[suit_index]);
    return buffer;
}

const char *getRandomColor() {
    const char *colors[] = {
        "\033[31m", // Red
        "\033[32m", // Green
        "\033[33m", // Yellow
        "\033[34m", // Blue
        "\033[35m", // Magenta
        "\033[36m", // Cyan
    };
    int num_colors = sizeof(colors) / sizeof(colors[0]);
    return colors[rand() % num_colors];
}

void printBanner() {
    const char *yellow = "\033[1;33m";
    const char *reset = "\033[0m";
 
    const char *card1 = getRandomColor();
    const char *card2 = getRandomColor();
 
    while (card1 == card2) {
        card2 = getRandomColor();
    }
 
    // clear screen
    printf("\033[2J\033[H");
    printf("    %s_________%s     %s_________%s\n", card1, reset, card2, reset);
    printf("   %s|A        |%s   %s|K        |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|    ^    |%s   %s|    %%    |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|________A|%s   %s|________K|%s\n", card1, reset, card2, reset);
    printf("           %sBlackJack%s\n\n", yellow, reset);
    const char *name = "        by Ryu Mendoza";
    for (int i = 0; name[i] != '\0'; i++) {
        printf("%s%c%s", getRandomColor(), name[i], reset);
    }
    printf("\n");
}

void error_exit(const char *message) {
    perror(message);
    exit(1);
}
 
const char *getColor(int choice) {
    const char *colors[] = {
        "\033[31m", // Red
        "\033[32m", // Green
        "\033[33m", // Yellow
        "\033[34m", // Blue
        "\033[35m", // Magenta
        "\033[36m", // Cyan
    };
    return colors[choice % 6];
}
 
void print_debug_info(Player *dealer, Player players[], int player_count) {
    printf("\n----------- Debug Info %d -----------\n", debugCounter++);
    printf("%s Score: %d, Hand: ", DEALER_STRING, dealer->score);
    for (int i = 0; i < dealer->hand_size; i++) {
        printf("%s, ", card_to_string(dealer->hand[i]));
    }
    printf("\n");
 
    for (int i = 0; i < player_count; i++) {
        printf("%s%s:\033[0m Score: %d, Hand: ", players[i].color, players[i].name, players[i].score);
        for (int j = 0; j < players[i].hand_size; j++) {
            printf("%s, ", card_to_string(players[i].hand[j]));
        }
        printf("\n");
    }
    printf("------------------------------------\n\n");
}

// Calculates the visible length of a string, ignoring ANSI escape codes.
int visible_strlen(const char *str) {
    int len = 0;
    int in_escape = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\033') {
            in_escape = 1;
        }
        if (!in_escape) {
            len++;
        }
        if (in_escape && str[i] == 'm') {
            in_escape = 0;
        }
    }
    return len;
}