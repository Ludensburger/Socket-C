#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1" // Change to the server's IP address if needed
#define PORT 8080
#define BUFFER_SIZE 1024

void printBanner() {
    const char *orange = "\033[38;5;208m";
    const char *red = "\033[31m";
    const char *yellow = "\033[1;33m";
    const char *reset = "\033[0m";

    // clear screen
    system("cls");

    printf("   %s_____%s     %s_____%s\n", orange, reset, red, reset);
    printf("  %s|A    |%s   %s|K    |%s\n", orange, reset, red, reset);
    printf("  %s|     |%s   %s|     |%s\n", orange, reset, red, reset);
    printf("  %s|  ^  |%s   %s|  %%  |%s\n", orange, reset, red, reset);
    printf("  %s|     |%s   %s|     |%s\n", orange, reset, red, reset);
    printf("  %s|____A|%s   %s|____K|%s\n", orange, reset, red, reset);
    printf("      %sBlackJack%s\n", yellow, reset);
    // printf("      BlackJack\n");
    printf("\n");
}

void error_exit(const char *message) {
    printf("%s. Error Code: %d\n", message, WSAGetLastError());
    WSACleanup();
    exit(1);
}

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int bytesRead;

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
        printf("\nServer:\n%s\n", buffer);

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
        printf("\nServer:\n%s\n", buffer);

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
            printf("\nServer:\n%s\n", buffer);

            // Check if the server is prompting for an action
            if (strstr(buffer, "Your turn: hit or stand?") != NULL) {
                // Get player action
                printf("Enter your action (hit/stand): ");
                fgets(buffer, BUFFER_SIZE, stdin);
                buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

                // Send action to server
                send(clientSocket, buffer, strlen(buffer), 0);
            }
        } else if (bytesRead == 0) {
            printf("Connection closed by server.\n");
            break; // Exit loop if connection closed
        } else {
            error_exit("recv failed");
        }
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}