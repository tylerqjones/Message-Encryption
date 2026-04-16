#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <netdb.h>

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
    inet_pton(AF_INET, "0.0.0.0", &serverAddress.sin_addr); // Run on any address on device. inet_pton converts the IP address from text to binary

    // Bind socket to address
    // bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if(bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
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

    // Accept a call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST]; // Client's remote name
    char svc[NI_MAXSERV]; // Service (port) the client is connect on

    int clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);
    if(clientSocket == -1) {
        std::cerr << "Problem with client connecting.\n";
        close(serverSocket);
        return -4;
    }

    // Close the listening socket

    close(serverSocket);

    memset(host, 0, NI_MAXHOST); // NI_MAXHOST = max size of host name <netdb.h>
    memset(svc, 0, NI_MAXSERV); // NI_MAXSERV = max size of service name <netdb.h>

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    if(result) {
        std::cout << host << " connected on port " << svc << '\n';
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST); // Converts the IP address from binary to text. Network to presentation.
        std::cout << host << " connected on port " << ntohs(client.sin_port) << '\n'; // Network to host short
    }

    // While receiving - display message, echo message
    char buffer[4096];
    while(true) {
        memset(buffer, 0, 4096);
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);

        if(bytesReceived == -1) {
            std::cerr << "Connection Issue.\n";
            break;
        }

        if(bytesReceived == 0) {
            std::cout << "Client disconnected.\n";
            break;
        }

        std::cout << "Received: " << std::string(buffer, 0, bytesReceived) << '\n';

        // Echo message back to client
        send(clientSocket, buffer, bytesReceived + 1, 0);
    }

    // Close socket
    close(clientSocket);

    return 0;
}