#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

const char *DEALER_STRING = "\033[1;33mDealer:\033[0m"; // Yellow color

const char *card_to_string(int card) {
    static char buffer[64]; // Increased buffer size
    const char *values[] = {"Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"};
    const char *suits[] = {"Spades", "Hearts", "Diamonds", "Clubs"};
    const char *colors[] = {"\033[38;5;208m", "\033[31m", "\033[36m", "\033[92m"}; // Orange, Red, Cyan, Light Green

    int value_index = card % 13;
    int suit_index = card / 13;

    snprintf(buffer, sizeof(buffer), "%s%s of %s\033[0m", colors[suit_index], values[value_index], suits[suit_index]);
    return buffer;
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

    return colors[choice];
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

void print_debug_info(Player *dealer, Player players[], int player_count) {

    int debugCounter = 1;

    printf("\n----------- Debug Info %d -----------\n", debugCounter++); // Add this line

    // Show dealer's cards and score
    printf("%s\n", DEALER_STRING); // Use the yellow colored dealer string
    printf("Score: %d\n", dealer->score);
    printf("Hand: ");
    for (int i = 0; i < dealer->hand_size; i++) {
        printf("%s", card_to_string(dealer->hand[i]));
        if (i < dealer->hand_size - 1) {
            printf(", ");
        }
    }
    printf("\n\n");

    // Show all players' cards
    for (int i = 0; i < player_count; i++) {
        printf("%s%s:\033[0m\n", players[i].color, players[i].name);
        printf("Score: %d\n", players[i].score);
        printf("Hand: ");
        for (int j = 0; j < players[i].hand_size; j++) {
            printf("%s", card_to_string(players[i].hand[j]));
            if (j < players[i].hand_size - 1) {
                printf(", ");
            }
        }
        printf("\n\n");
    }
    printf("-----------------------------------\n\n");
}

void printBanner() {
    const char *orange = "\033[38;5;208m";
    const char *red = "\033[31m";
    const char *yellow = "\033[1;33m";
    const char *reset = "\033[0m";

    const char *test1 = getRandomColor();
    const char *test2 = getRandomColor();

    while (test1 == test2) {
        test2 = getRandomColor();
    }

    // printf("Test1 Color: %s\n", test1);
    // printf("Test2 Color: %s\n", test2);

    // printf("Test1 Color: %sTest1\033[0m\n", test1);
    // printf("Test2 Color: %sTest2\033[0m\n", test2);

    // clear screen
    system("cls");

    printf("   %s_____%s     %s_____%s\n", test1, reset, test2, reset);
    printf("  %s|A    |%s   %s|K    |%s\n", test1, reset, test2, reset);
    printf("  %s|     |%s   %s|     |%s\n", test1, reset, test2, reset);
    printf("  %s|  ^  |%s   %s|  %%  |%s\n", test1, reset, test2, reset);
    printf("  %s|     |%s   %s|     |%s\n", test1, reset, test2, reset);
    printf("  %s|____A|%s   %s|____K|%s\n", test1, reset, test2, reset);
    printf("      %sBlackJack%s\n", yellow, reset);
    // printf("      BlackJack\n");
    printf("\n");
}

void error_exit(const char *message) {
    printf("%s. Error Code: %d\n", message, WSAGetLastError());
    WSACleanup();
    exit(1);
}