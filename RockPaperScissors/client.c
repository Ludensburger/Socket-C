#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib") // Link with Ws2_32.lib

#define PORT 8080
#define BUFFER_SIZE 1024

void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void to_title_case(char *str) {
    if (str[0]) {
        str[0] = toupper(str[0]);
    }
    for (int i = 1; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Game loop

    // Print banner

    // add a clear console command
    system("cls");

    printf("========================================\n");
    printf(" ! Welcome to ROCK - PAPER - SCISSORS !\n");
    printf("========================================\n\n");
    while (1) {

        printf("Enter your choice (Rock, Paper, Scissors) or 'exit' to quit: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

        // Convert input to lowercase
        to_lowercase(buffer);

        // Exit the game if the user types 'exit'
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Validate input
        if (strcmp(buffer, "rock") != 0 && strcmp(buffer, "paper") != 0 && strcmp(buffer, "scissors") != 0) {
            printf("Invalid choice. Please enter Rock, Paper, or Scissors.\n");
            continue;
        }

        // Convert input to title case before sending
        to_title_case(buffer);

        // Send choice to server
        send(sock, buffer, strlen(buffer), 0);

        // Receive result from server
        int bytesRead = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            printf("Result: %s\n", buffer);
        } else if (bytesRead == 0) {
            printf("Connection closed by server.\n");
            break; // Exit loop if connection closed
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
            break; // Exit loop if an error occurred
        }
    }

    // Cleanup
    closesocket(sock);
    WSACleanup();
    return 0;
}