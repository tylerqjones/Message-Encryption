#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <netdb.h>

// g++ client.cpp -o client
// ./client

int main() {
    // Create client socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket == -1) {
        std::cerr << "Failed to create socket.\n";
        return -1;
    }

    // Defining server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    if(connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to connect to server.\n";
        close(clientSocket);
        return -2;
    }

    std::cout << "Connected to server.\n";

    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);

    send(clientSocket, username.c_str(), username.length(), 0);

    std::cout << "Logged in as " << username << '\n';
    
    // Send data to the server
    char buffer[4096];
    std::string message;

    while(true) {
        std::cout << "Enter message (\"exit\" to close): ";
        std::getline(std::cin, message);

        send(clientSocket, message.c_str(), message.length(), 0);

        if(message == "exit") {
            break;
        }

        memset(buffer, 0, 4096);
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);
        if (bytesReceived > 0) {
            std::cout << "Server: " << std::string(buffer, 0, bytesReceived) << '\n';
        }
    }

    close(clientSocket);

    return 0;
}
