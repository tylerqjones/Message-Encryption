// Create a socket
// Bind the socket to an IP/port
// Mark the socket for listening in
// Accept a call
// Close the listening socket
// While receiving - display message, echo message
// Close socket

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>

// g++ server.cpp -o server
// ./server

int main() {
    // Creating the server socket - listening socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1) {
        std::cerr << "Failed to create socket.\n";
        return -1;
    }

    // Defining server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080); // Host to network short
    // serverAddress.sin_addr.s_addr = INADDR_ANY; // Run on any address on device. INADDR_ANY = 0.0.0.0
    inet_pton(AF_INET, "0.0.0.0", &serverAddress.sin_addr); // Run on any address on device. 

    // Bind socket to address
    // bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if(bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind socket to IP/port.\n";
        close(serverSocket);
        return -2;
    }

    // Listen for incoming connections
    // listen(serverSocket, 5);
    if(listen(serverSocket, SOMAXCONN) == -1) { // SOMAXCONN = max number of pending connections the queue will hold <sys/socket.h>
        std::cerr << "Failed to listen on socket.\n";
        close(serverSocket);
        return -3;
    }

    std::cout << "Server started.\n";

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