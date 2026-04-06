#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <unistd.h>

// g++ client.cpp -o client
// ./client

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
    char buffer[1024] = {0};
    std::string message;

    while(true) {
        std::cout << "Enter message (\"exit\" to close): ";
        std::getline(std::cin, message);

        send(clientSocket, message.c_str(), message.length(), 0);

        if(message == "exit") {
            break;
        }

        read(clientSocket, buffer, 1024);
        std::cout << buffer << '\n';
        memset(buffer, 0, sizeof(buffer));
    }

    close(clientSocket);

    return 0;
}
