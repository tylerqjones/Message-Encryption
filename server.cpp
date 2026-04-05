#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <unistd.h>

// g++ server.cpp -o server
// ./server

int main() {
    // Creating the server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Defining server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to address
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Listen for incoming connections
    listen(serverSocket, 5);

    // Accept client connection
    int clientSocket = accept(serverSocket, nullptr, nullptr);

    // Receive data from client
    char buffer[1024] = {0};
    while(true) {
        recv(clientSocket, buffer, sizeof(buffer), 0);

        if(strcmp(buffer, "exit") == 0) {
            std::cout << "Client disconnected." << '\n';
            break;
        }

        std::cout << "Message from client: " << buffer << std::endl;
        
        std::string receiveMessage = "Message received.";
        send(clientSocket, receiveMessage.c_str(), receiveMessage.length(), 0);

        memset(buffer, 0, sizeof(buffer));
    }


    close(serverSocket);

    return 0;
}