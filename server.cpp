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
#include <netdb.h>
#include <thread>

// g++ server.cpp -o server -pthread
// ./server

// Handles individual client connections
void handleClient(int clientSocket, sockaddr_in clientAddr) {
    char host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, host, INET_ADDRSTRLEN);

    char buffer[4096];
    
    // Gets username of client
    memset(buffer, 0, 4096);
    int bytesForName = recv(clientSocket, buffer, sizeof(buffer), 0);
    
    std::string username = std::string(buffer, 0, bytesForName);
    std::cout << host << " joined as " << username << ".\n";
    
    while(true) {
        memset(buffer, 0, 4096);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if(bytesReceived == -1) {
            std::cerr << "Connection Issue.\n";
            break;
        }

        if(bytesReceived == 0) {
            std::cout << username << " disconnected.\n";
            break;
        }

        std::string receivedData = std::string(buffer, 0, bytesReceived);
        std::cout << "[" << host << "] " << username << ": " << receivedData << '\n';

        // Echo message back to client
        send(clientSocket, buffer, bytesReceived, 0);
    }
    close(clientSocket);
}

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
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Run on any address on device. INADDR_ANY = 0.0.0.0

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

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if(clientSocket != -1) {
            // Start a new thread for each client and detach it
            std::thread(handleClient, clientSocket, clientAddr).detach();
            }
    }

    close(serverSocket);

    return 0;
}