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
#include <bits/stdc++.h>

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

struct ClientInfo {
    long long clientPublicExponent, clientPrivateExponent, clientModulus;
    long long targetPublicExponent, targetModulus;
    bool isEncrypted = false;
};

ClientInfo clientInfo;
std::mutex clientMutex;

// Computes (base^expo) mod m
long long power(long long base, long long expo, long long m) {
    long long res = 1;
    base = base % m;
    while(expo > 0) {
        if(expo & 1) {
            res = (res * base) % m;
        }
        base = (base * base) % m;
        expo = expo / 2;
    }
    return res;
}

// Finds modular inverse of e modulo phi(n)
// Uses Hit and Trial Method
long long modInverse(long long publicExponent, long long phi) {
    for(long long d = 2; d < phi; d++) {
        if((publicExponent * d) % phi == 1)
            return d;
    }
    return -1;
}

bool isPrime(int num) {
    if(num < 2) {
        return false;
    }
    for(int i = 2; i * i <= num; i++) {
        if((num % i) == 0) {
            return false;
        }
    }
    return true;
}

// Generates a radnom prime number with a min and max
int generateRandomPrime(int min, int max) {
    int prime = rand() % (max - min + 1) + min;
    while(!isPrime(prime)) {
        prime++;
        if(prime > max) {
            prime = min;
        }
    }
    return prime;
}

// RSA Key Generation
void generateKeys(long long &publicExponent, long long &privateExponent, long long &modulus) {
    long long p = generateRandomPrime(1000, 10000);
    long long q = generateRandomPrime(1000, 10000);
    
    modulus = p * q;
    long long phi = (p - 1) * (q - 1);

    // Choose e, where 1 < e < phi(n) and gcd(e, phi(n)) == 1
    for(publicExponent = 2; publicExponent < phi; publicExponent++) {
        if(std::gcd(publicExponent, phi) == 1) {
            break;
        }
    }
    
    // Compute d such that e * d ≡ 1 (mod phi(n))
    privateExponent = modInverse(publicExponent, phi);
}

// Encrypt message using public key (e, n)
long long encrypt(long long messageChar, long long publicExponent, long long modulus) {
    return power(messageChar, publicExponent, modulus);
}

// Decrypt message using private key (d, n)
long long decrypt(long long ciphertext, long long privateExponent, long long modulus) {
    return power(ciphertext, privateExponent, modulus);
}

// Listener thread. Handles received server messages.
void receiveMessages(int socket) {
    char buffer[4096];
    while(true) {
        memset(buffer, 0, 4096);
        int bytesReceived = recv(socket, buffer, 4096, 0);
        
        if(bytesReceived <= 0) {
            std::cout << "\n[Disconnected from server]\n";
            exit(0);
        }

        std::string receivedData(buffer, 0, bytesReceived);

        // Assigns the target's key
        int keyPos = receivedData.find("[KEY]");
        if(keyPos != -1) {
            int spaceAfterKey = receivedData.find(' ', keyPos);
            int spaceAfterUsername = receivedData.find(' ', spaceAfterKey + 1); // Substr after [KEY]
            int spaceAfterExponent = receivedData.find(' ', spaceAfterUsername + 1);
            
            std::lock_guard<std::mutex> lock(clientMutex);
            clientInfo.targetPublicExponent = std::stoll(receivedData.substr(spaceAfterUsername + 1, spaceAfterExponent - spaceAfterUsername - 1));
            clientInfo.targetModulus = std::stoll(receivedData.substr(spaceAfterExponent + 1));
            clientInfo.isEncrypted = true;

            std::cout << receivedData;
            continue;
        }

        // For when ending encryption
        if(receivedData.find("[STOPKEY]") != -1) {
            std::lock_guard<std::mutex> lock(clientMutex);
            std::cout << receivedData;
            clientInfo.isEncrypted = false;
            continue;
        }

        // Decrypts received message
        int encryptionPos = receivedData.find("[ENCRYPTED]");
        if(encryptionPos != -1) {
            std::string decryptedMessage = "";
            std::string encryptedMessage = receivedData.substr(encryptionPos + 12); // Substr after [ENCRYPTED]

            int startMsg = 0;
            while(startMsg < encryptedMessage.length()) {
                int endMsg = encryptedMessage.find(' ', startMsg);
                std::string encryptedChar;

                if(endMsg == -1) {
                    encryptedChar = encryptedMessage.substr(startMsg);
                    startMsg = encryptedMessage.length();
                } else {
                    encryptedChar = encryptedMessage.substr(startMsg, endMsg - startMsg);
                    startMsg = endMsg + 1;
                }

                if(!encryptedChar.empty() && encryptedChar != "\n") {
                    decryptedMessage += static_cast<char>(decrypt(std::stoll(encryptedChar), clientInfo.clientPrivateExponent, clientInfo.clientModulus));
                }
            }

            std::lock_guard<std::mutex> lock(clientMutex);
            std::cout << receivedData.substr(0, encryptionPos) << decryptedMessage << "\n";
        } else {
            std::lock_guard<std::mutex> lock(clientMutex);
            std::cout << receivedData;
        }
    }
}

int main() {
    srand((time(0)));

    generateKeys(clientInfo.clientPublicExponent, clientInfo.clientPrivateExponent, clientInfo.clientModulus);

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

    {
    std::lock_guard<std::mutex> lock(clientMutex);
    std::string loginInfo = username + " " + std::to_string(clientInfo.clientPublicExponent) + " " + std::to_string(clientInfo.clientModulus);
    send(clientSocket, loginInfo.c_str(), loginInfo.length(), 0);
    std::cout << "Logged in as " << username << '\n';
    }

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

        std::lock_guard<std::mutex> lock(clientMutex);

        // Encrypts true ans long as not command.
        if(clientInfo.isEncrypted && message[0] != '/') {
            std::string encryptedMessage = "[ENCRYPTED] ";
            for(char c : message) {
                long long cipherChar = encrypt(static_cast<long long>(c), clientInfo.targetPublicExponent, clientInfo.targetModulus);
                encryptedMessage += std::to_string(cipherChar) + " ";
            }
            send(clientSocket, encryptedMessage.c_str(), encryptedMessage.length(), 0);
        } else {
            send(clientSocket, message.c_str(), message.length(), 0);
            if(message == "/stopmsg") {
                clientInfo.isEncrypted = false;
            }
        }
    }
    close(clientSocket);

    return 0;
}
