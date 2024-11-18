#include "stack.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
        cards[i] = i; // Generate card values from 0 to 51
    }
    shuffleCards(cards, STACK_SIZE);
    initializeStack(stack);
    for (int i = 0; i < STACK_SIZE; i++) {
        push(stack, cards[i]);
    }
}

void resetAndFillStack(Stack *stack) {
    int cards[STACK_SIZE];
    for (int i = 0; i < STACK_SIZE; i++) {
        cards[i] = i; // Generate card values from 0 to 51
    }
    shuffleCards(cards, STACK_SIZE);
    initializeStack(stack);
    for (int i = 0; i < STACK_SIZE; i++) {
        push(stack, cards[i]);
    }
}

void cleanStack(Stack *stack) {
    initializeStack(stack);
}

void printStack(Stack *stack) {
    srand(time(NULL));     // Seed the random number generator
    int cardPartition = 2; // Adjust the division of cards in the server if needed
    for (int i = 0; i <= stack->top; i++) {
        if (i % 10 == 0 && i != 0) {
            printf("\n"); // Print a blank line for a new stack
        }
        if (i % cardPartition == 0) {
            printf("-------Stack %d-------\n", (i / 10) + 1); // Stack Bar
        }
        printf("%s\n", card_to_string(stack->cards[i])); // Cards
    }
    printf("\n");
}
