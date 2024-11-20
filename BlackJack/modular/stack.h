#ifndef STACK_H
#define STACK_H

#define STACK_SIZE 52

typedef struct {
    int cards[STACK_SIZE];
    int top;
} Stack;

void initializeStack(Stack *stack);
int isEmpty(Stack *stack);
int isFull(Stack *stack);
void push(Stack *stack, int card);
int pop(Stack *stack);
void shuffleCards(int *cards, int size);
void fillStack(Stack *stack);
void resetAndFillStack(Stack *stack);
void cleanStack(Stack *stack);
void printStack(Stack *stack, int player_count);

#endif // STACK_H