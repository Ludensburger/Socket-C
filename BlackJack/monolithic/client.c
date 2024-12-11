/*

    Author: Ryu Mendoza
    Data Communication and Networking 1

    Overview:
    This program demonstrates a Many-to-One connection setup for socket programming.
    - One main server (also referred to as "Player 1" or "Primary Player") will host the session.
        In this case, it is most likely that Player 1 will run/host the server.
    - Multiple clients (other computers) will connect to the server using their IP addresses.

    So its a Many to One connection.

    Requirements:
    1. The **host machine** (server) must set its IPV4 address as the `SERVER_IP`.
        Example: If I am the host, I will set my IPV4 address as `192.168.0.1`.

    2. All **client machines** must have IPV4 addresses in the same local network as the `SERVER_IP`.
        Example:
        - Ensure your IPV4 address matches the network of the `SERVER_IP`.
            Specifically, this means your address must be within the usable host range, starting from the first usable address after the network identifier up to the address before the broadcast address.
        - Compile the `client.c` file: `gcc client.c -o client`.
        - Run the client executable: `./client` or `client.exe`.


    Instructions:
    1. **For the Host (Server):**
        - Set your machine's IPV4 address as `SERVER_IP` in the source code.
        - Compile the `server.c` file: `gcc server.c -o server`.
        - Run the server executable: `./server` or `server.exe`.

    2. **For the Clients:**
        - Ensure your IPV4 address matches the network of the `SERVER_IP`.
            Specifically, this means your address must be within the usable host range, starting from the first usable address after the network identifier up to the address before the broadcast address.
        - Compile the `client.c` file: `gcc client.c -o client`.
        - Run the client executable: `./client` or `client.exe`.


    Network Troubleshooting:
    - If a client cannot connect:
        1. Confirm the server is running and listening on the correct IP and port.
        2. Check that all devices are on the same network.
        3. Ensure firewalls or antivirus programs are not blocking the connection.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_IP "127.0.0.1" // Change to the server's IP address if needed

// Change for multi-player with other computers
// #define SERVER_IP "192.168.0.1"

#define PORT 8080 // Change to the correct port if needed
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
    printf("    %s_________%s     %s_________%s\n", card1, reset, card2, reset);
    printf("   %s|A        |%s   %s|K        |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|    ^    |%s   %s|    %%    |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|         |%s   %s|         |%s\n", card1, reset, card2, reset);
    printf("   %s|________A|%s   %s|________K|%s\n", card1, reset, card2, reset);
    printf("           %sBlackJack%s\n\n", yellow, reset);
    const char *name = "        by Ryu Mendoza";
    for (int i = 0; name[i] != '\0'; i++) {
        printf("%s%c%s", getRandomColor(), name[i], reset);
    }
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
