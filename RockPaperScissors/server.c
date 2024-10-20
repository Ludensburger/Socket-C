#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib") // Link with Ws2_32.lib

#define PORT 8080
#define BUFFER_SIZE 1024

const char *choices[] = {"Rock", "Paper", "Scissors"};

void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

const char *determine_winner(const char *client_choice, char *server_choice) {
    int random_choice = rand() % 3;                // Random choice for the server
    strcpy(server_choice, choices[random_choice]); // Copy server choice to output
    printf("Server choice: %s\n", server_choice);

    // Convert choices to lowercase for comparison
    char client_choice_lower[BUFFER_SIZE];
    char server_choice_lower[BUFFER_SIZE];
    strcpy(client_choice_lower, client_choice);
    strcpy(server_choice_lower, server_choice);
    to_lowercase(client_choice_lower);
    to_lowercase(server_choice_lower);

    // Determine the winner
    if (strcmp(client_choice_lower, server_choice_lower) == 0) {
        return "It's a draw!";
    } else if ((strcmp(client_choice_lower, "rock") == 0 && strcmp(server_choice_lower, "scissors") == 0) ||
               (strcmp(client_choice_lower, "paper") == 0 && strcmp(server_choice_lower, "rock") == 0) ||
               (strcmp(client_choice_lower, "scissors") == 0 && strcmp(server_choice_lower, "paper") == 0)) {
        return "You win!";
    } else {
        return "You lose!";
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    char server_choice[20]; // Buffer for server's choice

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Waiting for connections...\n");

    // Accept a client socket
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET) {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Client connected.\n");

    // Game loop
    while (1) {
        // Receive choice from client
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            printf("Received from client: %s\n", buffer);

            // Exit the game if the client sends 'exit'
            if (strcmp(buffer, "exit") == 0) {
                printf("Client has exited the game.\n");
                break;
            }

            // Determine the winner
            const char *result = determine_winner(buffer, server_choice);

            // Format response to include the server's choice
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "%s (Computer picked: %s)\n", result, server_choice);

            send(clientSocket, response, strlen(response), 0); // Send result to client
        } else if (bytesRead == 0) {
            printf("Connection closed by client.\n");
            break; // Exit loop if connection closed
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
            break; // Exit loop if an error occurred
        }
    }

    // Cleanup
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}