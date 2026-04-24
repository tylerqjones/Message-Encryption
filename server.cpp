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
#include <vector>
#include <mutex>
#include <bits/stdc++.h>

// g++ server.cpp -o server -pthread
// ./server

const int PORT = 8080;

struct ClientInfo {
    std::string username;
    int socket;
    long long publicExponent;
    long long modulus;
};

std::vector<ClientInfo> activeClients;
std::mutex serverMutex;

// Sends message to a specific socket
void sendMessage(int socket, const std::string& msg) {
    send(socket, msg.c_str(), msg.length(), 0);
}

void displayUsers(int socket) {
    std::string userList = "[SERVER] Online users: ";
    serverMutex.lock();
    for(int i = 0; i < activeClients.size(); i++) {
        userList += activeClients[i].username;
        if(i != activeClients.size() - 1) {
        userList += ", ";
        }
    }
    sendMessage(socket, userList + '\n');
    serverMutex.unlock();
}

// Handles individual client connections
void handleClient(int clientSocket, sockaddr_in clientAddr) {
    char host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, host, INET_ADDRSTRLEN);

    char buffer[4096];
    
    // Gets username of client
    memset(buffer, 0, 4096);
    int bytesForName = recv(clientSocket, buffer, sizeof(buffer), 0);
    if(bytesForName <= 0) {
        std::cout << "[SERVER] Username error.\n";
        close(clientSocket);
        return;
    }
    std::string loginData(buffer, 0, bytesForName);

    int firstSpace = loginData.find(' ');
    int secondSpace = loginData.find(' ', firstSpace + 1);

    std::string username = loginData.substr(0, firstSpace);
    long long clientPublicExponent = std::stoll(loginData.substr(firstSpace + 1, secondSpace - firstSpace - 1)); // string to long long
    long long clientModulus = std::stoll(loginData.substr(secondSpace + 1));

    // Check if username already exists
    serverMutex.lock();
    for(int i = 0; i < activeClients.size(); i++) {
        if(activeClients[i].username == username) {
            sendMessage(clientSocket, "[SERVER] Username already taken.\n");
            serverMutex.unlock();
            close(clientSocket);
            return;
        }
    }
    activeClients.push_back({username, clientSocket, clientPublicExponent, clientModulus});
    serverMutex.unlock();

    std::cout << "[SERVER] " << host << " joined as " << username << ".\n";
    sendMessage(clientSocket, "[SERVER] Type '/list' to see user list. Type '/msg <username>' to message a user. Type '/stopmsg' to stop messaging user. Type '/exit' to exit.\n");
    
    std::string targetUser = ""; // Target user to message

    while(true) {
        memset(buffer, 0, 4096);
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if(bytesReceived <= 0) {
            std::cout << "[SERVER] " << username << " disconnected.\n";
            break;
        }

        // Contains received message
        std::string receivedData = std::string(buffer, 0, bytesReceived);

        if(receivedData == "/list") {
            displayUsers(clientSocket);
            continue;
        } else if(receivedData.find("/msg ") == 0) {
            targetUser = receivedData.substr(5);

            bool found = false;
            long long targetPublicExponent, targetModulus;

            serverMutex.lock();
            for(int i = 0; i < activeClients.size(); i++) {
                if(activeClients[i].username == targetUser) {
                    found = true;
                    targetPublicExponent = activeClients[i].publicExponent;
                    targetModulus = activeClients[i].modulus;
                    break;
                }
            }
            serverMutex.unlock();

            if(!found) {
                sendMessage(clientSocket, "[SERVER] " + targetUser + " not found.\n");
                targetUser = "";
                continue;
            }

            sendMessage(clientSocket, "[SERVER] Now messaging " + targetUser + ".\n");

            std::string keyCommand = "[KEY] " + targetUser + " " + std::to_string(targetPublicExponent) + " " + std::to_string(targetModulus) + "\n";
            sendMessage(clientSocket, keyCommand);
            continue;

        } else if(receivedData.find("/stopmsg") == 0) {
            sendMessage(clientSocket, "[STOPKEY]\n");
            sendMessage(clientSocket, "[SERVER] No longer messaging " + targetUser + ".\n");
            targetUser = "";
            continue;
        }

        if(targetUser.empty()) {
            sendMessage(clientSocket, "[SERVER] Use /msg <username> to message.\n");
            continue;
        }

        std::cout << "[" << host << "] " << username << " to " << targetUser << ": " << receivedData << '\n';

        // Sending a direct message
        bool found = false;
        serverMutex.lock();
        for(int i = 0; i < activeClients.size(); i++) {
            if(activeClients[i].username == targetUser) {
                std::string directMsg = username + ": " + receivedData + "\n";
                sendMessage(activeClients[i].socket, directMsg);
                found = true;
                break; 
            }
        }
        if(!found) {
            sendMessage(clientSocket, "[SERVER] " + targetUser + " not found.");
            targetUser = "";
        }
        serverMutex.unlock();
    }

    // Remove from active clients once disconnected
    serverMutex.lock();
    for(int i = 0; i < activeClients.size(); i++) {
        if(activeClients[i].socket == clientSocket) {
            activeClients.erase(activeClients.begin() + i);
            break; 
        }
    }
    serverMutex.unlock();

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
    serverAddress.sin_port = htons(PORT); // Host to network short
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

    std::cout << "Server started on port " << PORT << ".\n";

    while(true) {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if(clientSocket != -1) {
            // Start a new thread for each client and detach it
            std::thread newClient(handleClient, clientSocket, clientAddr);
            newClient.detach();
        }
    }
    close(serverSocket);

    return 0;
}