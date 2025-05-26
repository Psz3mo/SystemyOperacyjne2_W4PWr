#include <iostream>
#include <winsock2.h>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <random>

#define PORT 7000

struct sockaddr_in serverAddr;

void handleClient(SOCKET clientSocket); // Function to handle client communication
void broadcast(const std::string& message); // Function to broadcast messages to all clients
void serverInputThread(); // Function to handle server input (for sending messages to clients)
std::string getRandomColorCode(); // Function to get a random color code

std::mutex clientsMutex; // Mutex for thread safety
std::vector<SOCKET> clients; // Vector to store connected client sockets
std::map<SOCKET, std::string> clientNames; // Map to store client names
std::map<SOCKET, std::string> clientColors; // Map to store client colors
std::mutex printMutex; // Mutex for printing messages

SOCKET serverSocket; // Socket for the server

int main(int argc, char* argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) < 0){
        std::cout << "Error initializing Winsock" << std::endl;
        return 1;
    }

    // Create a socket
    serverSocket  = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0){
        std::cout << "Error creating socket" << std::endl;
        return 1;
    }

    // Set up the server address structure
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_port = htons(PORT); // Port number
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Any local address
    memset(&(serverAddr.sin_zero), 0, 8); // Zero out the rest of the structure

    // Bind the socket to the address and port
    int ret = bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0){
        std::cout << "Error binding socket" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    ret = listen(serverSocket, SOMAXCONN);
    if (ret < 0){
        std::cout << "Error listening on socket" << std::endl;
        return 1;
    }
    
    // Accept incoming connections in a loop
    std::cout << "Server is listening on port " << PORT << std::endl;

    // Create a thread to handle server input (for sending messages to clients)
    std::thread inputThread(serverInputThread);
    inputThread.detach();

    // Accept clients in a loop
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            int err = WSAGetLastError();
            if (err == WSAENOTSOCK || err == WSAEINVAL) { // Invalid socket error
                break;
            } 
            // Error accepting client connection
            continue;
        }
    
        // Create new thread for client
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }
    closesocket(serverSocket); // Close the server socket
    WSACleanup();
    return 0;
}

// Function to handle client communication
// This function runs in a separate thread for each client
// It receives messages from the client and prints them to the console
// It also handles disconnection by closing the client socket
void handleClient(SOCKET clientSocket) {
    // Add the client to the list of connected clients
    clientsMutex.lock();
    clients.push_back(clientSocket);
    clientsMutex.unlock();

    // Receive the client's name
    char nameBuffer[100];
    int nameLen = recv(clientSocket, nameBuffer, sizeof(nameBuffer) - 1, 0);
    if (nameLen <= 0) {
        closesocket(clientSocket);
        return;
    }
    nameBuffer[nameLen] = '\0';
    std::string username = nameBuffer;
    
    clientsMutex.lock();
    clientNames[clientSocket] = username;
    clientColors[clientSocket] = getRandomColorCode();
    clientsMutex.unlock();

    broadcast("\033[93m" + username + " has joined the chat.\033[0m"); // Notify other clients

    // Process incoming messages from the client
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            broadcast("\033[93m" + username + " has left the chat.\033[0m"); // Notify other clients
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string msg = buffer;
        msg = clientColors[clientSocket] + username + "\033[0m: " + msg; // Prepend the username to the message
        broadcast(msg);
    }

    // Remove the client from the list of connected clients
    clientsMutex.lock();
    clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
    clientNames.erase(clientSocket);
    clientColors.erase(clientSocket);
    clientsMutex.unlock();

    closesocket(clientSocket);
}

// Function to broadcast messages to all connected clients
// This function is called when a client sends a message
// It sends the message to all other clients except the sender
void broadcast(const std::string& message) {
    clientsMutex.lock();
    for (SOCKET client : clients) {
        send(client, message.c_str(), message.length(), 0);
    }
    clientsMutex.unlock();
    printMutex.lock();
    std::cout << message << std::endl; // Print the message to the server console
    printMutex.unlock();
}

void serverInputThread() {
    std::string message;
    while (true) {
        std::getline(std::cin, message); // Get input from the server console
        if (message == "/exit") { // Exit command
            broadcast("\033[93mServer is shutting down...\033[0m"); 

            clientsMutex.lock();
            for (SOCKET client : clients) {
                closesocket(client);
            }
            clients.clear();
            clientsMutex.unlock();

            closesocket(serverSocket); // Close the server socket
            break;
        }
        if (message.empty()) continue;
        else {
            message = "[\033[31mServer\033[0m]: " + message;
            broadcast(message);
        }        
    }
}

std::string getRandomColorCode() {
    static std::vector<std::string> colors = { // List of ANSI color codes
        "\033[31m", "\033[32m", "\033[33m", "\033[34m",
        "\033[35m", "\033[36m", "\033[91m", "\033[92m",
        "\033[93m", "\033[94m", "\033[95m", "\033[96m"
    };
    // Randomly select a color code from the list
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, colors.size() - 1);

    return colors[dis(gen)];
}