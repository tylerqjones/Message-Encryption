#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

int main() {
    // Create client socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Defining server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Send data to the server
    const char* message = "Hello, server!";
    send(clientSocket, message, strlen(message), 0);

    close(clientSocket);

    return 0;
}