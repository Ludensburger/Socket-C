#include "stack.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void initializeStack(Stack *stack) {
    stack->top = -1;
}

int isEmpty(Stack *stack) {
    return stack->top == -1;
}

int isFull(Stack *stack) {
    return stack->top == STACK_SIZE - 1;
}

void push(Stack *stack, int card) {
    if (isFull(stack)) {
        printf("Stack is full. Cannot push card %d\n", card);
        return;
    }
    stack->cards[++stack->top] = card;
}

int pop(Stack *stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty. Cannot pop card\n");
        return -1;
    }
    return stack->cards[stack->top--];
}

void shuffleCards(int *cards, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }
}

void fillStack(Stack *stack) {
    int cards[STACK_SIZE];
    for (int i = 0; i < STACK_SIZE; i++) {
        cards[i] = i;
    }

    shuffleCards(cards, STACK_SIZE);
    initializeStack(stack);

    for (int i = 0; i < STACK_SIZE; i++) {
        push(stack, cards[i]);
    }
}

void resetAndFillStack(Stack *stack) {
    cleanStack(stack);
    fillStack(stack);
}

void cleanStack(Stack *stack) {
    initializeStack(stack);
}

void printStack(Stack *stack, int player_count) {
    const int column_width = 25;
    printf("\n==============================================================================\n");
    printf("                      Initial Shuffled Deck (%d cards)\n", stack->top + 1);
    printf("==============================================================================\n");

    // Print cards in neat columns
    int count = 0;
    for (int i = 0; i <= stack->top; i++) {
        char temp_buffer[100];
        const char *card_str = card_to_string(stack->cards[i]);
        if (i == stack->top) {
            snprintf(temp_buffer, sizeof(temp_buffer), "%s (<- Next Card)", card_str);
            printf("%s", temp_buffer);
        } else {
            snprintf(temp_buffer, sizeof(temp_buffer), "%s", card_str);
            printf("%s", temp_buffer);
        }
        printf("%*s", column_width - visible_strlen(temp_buffer), ""); // Pad with spaces
        if (++count % 3 == 0) {
            printf("\n");
        }
    }
    printf("\n==============================================================================\n\n");
}