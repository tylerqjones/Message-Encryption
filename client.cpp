#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <netdb.h>
#include <thread>
#include <mutex>

// g++ client.cpp -o client
// ./client

// Choose user to message
// Listener thread
// Function for output called with execpv. Mutual exclusion

// Client sends encrypted message to server
// Server sends encrypted message to other client
// Each client has a public key and private key
// RSA encryption algorithm

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1";

std::mutex clientMutex;

// Listener thread
void receiveMessages(int socket) {
    char buffer[4096];
    while(true) {
        memset(buffer, 0, 4096);
        int bytesReceived = recv(socket, buffer, 4096, 0);
        
        if (bytesReceived <= 0) {
            std::cout << "\n[Disconnected from server]\n";
            exit(0);
        }

        std::string receivedData = std::string(buffer, 0, bytesReceived);
        
        clientMutex.lock();
        std::cout << receivedData;
        clientMutex.unlock();
    }
}

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
    serverAddress.sin_port = htons(PORT);
    // serverAddress.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr);

    // Connect to server
    if(connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to connect to server.\n";
        close(clientSocket);
        return -2;
    }

    std::cout << "Connected to server.\n";

    std::string username;
    do {
        std::cout << "Enter your username: ";
        std::getline(std::cin, username);
    } while(username.empty());

    send(clientSocket, username.c_str(), username.length(), 0);
    std::cout << "Logged in as " << username << '\n';

    std::thread listener(receiveMessages, clientSocket);
    listener.detach();
    
    // Send data to the server
    std::string message;
    while(true) {

        std::getline(std::cin, message);

        if(message.empty()) {
            continue;
        }

        if(message == "/exit") {
            break;
        }

        clientMutex.lock();
        send(clientSocket, message.c_str(), message.length(), 0);
        clientMutex.unlock();
    }

    close(clientSocket);

    return 0;
}
