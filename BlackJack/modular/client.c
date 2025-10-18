#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 4096      // Increased buffer size
#define SERVER_IP "127.0.0.1" // Use this for local play

// Helper function to convert a string to lowercase for case-insensitive comparison
void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Function to get and validate user input for actions
void get_validated_input(char *buffer, size_t size) {
    while (1) {
        fgets(buffer, size, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
        to_lowercase(buffer);

        if (strcmp(buffer, "hit") == 0 || strcmp(buffer, "stand") == 0) {
            return; // Valid input
        }
        printf("Invalid input. Please enter 'hit' or 'stand': ");
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int bytesRead;

    // Seed the random number generator
    srand(time(NULL));

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        error_exit("Socket creation failed");
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        error_exit("Connection to server failed");
    }

    printf("Connected to server.\n");
    printBanner();

    // Receive game mode prompt from server
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the string
        printf("\nServer:\n%s", buffer);

        // Send game mode selection to server
        // printf("Pick from 1-5: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        send(clientSocket, buffer, strlen(buffer), 0);
    } else {
        error_exit("Failed to receive game mode prompt from server");
    }

    // Receive player name prompt from server
    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the string
        printf("\nServer:\n%s", buffer);

        // Send player name to server
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        send(clientSocket, buffer, strlen(buffer), 0);
    } else {
        error_exit("Failed to receive player name prompt from server");
    }

    // Game loop
    while (1) {
        // Receive message from server
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string

            // Check if the server is prompting for an action
            if (strstr(buffer, "Your turn: hit or stand?") != NULL) {
                printf("\n\nServer:\n%s", buffer);
                // Get player action
                printf("\nEnter your action (hit/stand): ");
                get_validated_input(buffer, BUFFER_SIZE);
                // Send action to server
                send(clientSocket, buffer, strlen(buffer), 0);
            } else if (strstr(buffer, "Play again? (yes/no):") != NULL) {
                // Split the final result from the "Play again?" prompt.
                char *prompt_location = strstr(buffer, "\nPlay again? (yes/no):");
                *prompt_location = '\0'; // Cut the string to separate the result.

                printf("\n\nServer:\n%s", buffer); // Print just the game result.
                printf("\n%s", prompt_location + 1); // Print the "Play again?" prompt.

                printf("\nEnter your choice (yes/no): ");
                fgets(buffer, BUFFER_SIZE, stdin);
                buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
                to_lowercase(buffer);
                send(clientSocket, buffer, strlen(buffer), 0);

                if (strcmp(buffer, "no") == 0) {
                    break; // Exit loop if user chooses not to play again
                }
            } else if (strcmp(buffer, "NEW_ROUND") == 0) {
                // Server is starting a new round.
                printBanner();
            } else {
                printf("\n\nServer:\n%s", buffer); // Handle any other messages
            }
        } else if (bytesRead == 0) {
            printf("Connection closed by server.\n");
            break; // Exit loop if connection closed
        } else {
            error_exit("recv failed");
        }
    }

    // Cleanup
    close(clientSocket);

    printBanner(); // Show the banner one last time at the end.
    printf("%s\nGame Over.\n\n%s", getColor(0), "\033[0m");

    // Wait for user input before exiting
    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}