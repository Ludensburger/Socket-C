#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib") // Link with Ws2_32.lib

#define PORT 8080
#define BUFFER_SIZE 1024

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
    while (1) {
        printf("Enter your choice (Rock, Paper, Scissors) or 'exit' to quit: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

        // Exit the game if the user types 'exit'
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

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