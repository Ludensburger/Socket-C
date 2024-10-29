#ifndef UTILS_H
#define UTILS_H

#define BUFFER_SIZE 4096 // Increased buffer size

extern const char *DEALER_STRING;

const char *card_to_string(int card);
const char *getRandomColor();
void printBanner();
void error_exit(const char *message);

#endif // UTILS_H