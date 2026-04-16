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

    // Defining server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Accept a call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST]; // Client's remote name
    char svc[NI_MAXSERV]; // Service (port) the client is connect on

    memset(host, 0, NI_MAXHOST); // NI_MAXHOST = max size of host name <netdb.h>
    memset(svc, 0, NI_MAXSERV); // NI_MAXSERV = max size of service name <netdb.h>

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    if(result) {
        std::cout << host << " connected on port " << svc << '\n';
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST); // Converts the IP address from binary to text. Network to presentation.
        std::cout << host << " connected on port " << ntohs(client.sin_port) << '\n'; // Network to host short
    }

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
