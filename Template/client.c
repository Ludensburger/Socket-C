#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080

int main() {

    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    const char *message = "Hello Server, this is the Client!";

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a client socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Client socket created.\n");

    // Prepare the sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Connected to server.\n");

    // Send data to the server
    send(clientSocket, message, strlen(message), 0);

    // Receive a reply from the server
    int recvSize = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (recvSize == SOCKET_ERROR) {
        printf("recv failed. Error Code: %d\n", WSAGetLastError());
    } else {
        buffer[recvSize] = '\0';
        printf("Server reply: %s\n", buffer);
    }

    // Clean up
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
