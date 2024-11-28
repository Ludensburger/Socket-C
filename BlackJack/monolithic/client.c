#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1" // Change to the server's IP address if needed
#define PORT 8080             // Change to the correct port if needed
#define BUFFER_SIZE 1024

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

// Function to generate a random color code
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
    const char *orange = "\033[38;5;208m";
    const char *red = "\033[31m";
    const char *yellow = "\033[1;33m";
    const char *reset = "\033[0m";
    const char *offSetTab = "\t\t\t";

    const char *card1 = getRandomColor();
    const char *card2 = getRandomColor();

    while (card1 == card2) {
        card2 = getRandomColor();
    }

    // clear screen
    system("cls");
    printf("%s", offSetTab);
    printf("   %s_________%s     %s_________%s\n%s", card1, reset, card2, reset, offSetTab);
    printf("  %s|A        |%s   %s|K        |%s\n%s", card1, reset, card2, reset, offSetTab);
    printf("  %s|         |%s   %s|         |%s\n%s", card1, reset, card2, reset, offSetTab);
    printf("  %s|         |%s   %s|         |%s\n%s", card1, reset, card2, reset), offSetTab;
    printf("  %s|    ^    |%s   %s|    %%    |%s\n%s", card1, reset, card2, reset, offSetTab);
    printf("  %s|         |%s   %s|         |%s\n%s", card1, reset, card2, reset, offSetTab);
    printf("  %s|         |%s   %s|         |%s\n%s", card1, reset, card2, reset, offSetTab);
    printf("  %s|________A|%s   %s|________K|%s\n%s", card1, reset, card2, reset, offSetTab);
    printf("      %sBlackJack%s\n", yellow, reset);
    printf("\n");
}

void error_exit(const char *message) {
    printf("%s. Error Code: %d\n", message, WSAGetLastError());
    WSACleanup();
    exit(1);
}

// Helper method to convert a string to lowercase
void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int bytesRead;

    // Seed the random number generator
    srand(time(NULL));

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error_exit("Failed to initialize Winsock");
    }

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        error_exit("Socket creation failed");
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
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

            printBanner(); // Print the banner

            printf("\nServer:\n%s", buffer);

            // Check if the server is prompting for an action
            if (strstr(buffer, "\nYour turn: hit or stand?\nEnter your action (hit/stand): ") != NULL) {
                // No need to print additional prompt since it's in server message
                fflush(stdout);

                // Get input on the same line
                fgets(buffer, BUFFER_SIZE, stdin);
                buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

                // Convert input to lowercase
                to_lowercase(buffer);

                // Send action to server
                send(clientSocket, buffer, strlen(buffer), 0);
            }
        } else if (bytesRead == 0) {
            printf("\nConnection closed by server.\n");
            break; // Exit loop if connection closed
        } else {
            error_exit("recv failed");
        }
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    printf("%s\nGame Over.\n\n%s", getColor(0), "\033[0m");

    // Wait for user input before exiting
    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}
