#include <iostream>
#include <thread>
#include <winsock2.h>
#include <string>
#include <vector>
#include <mutex>
#include <windows.h>
#include <conio.h>
#include <algorithm>
#include <sstream>

#define PORT 7000
#define SERVER "127.0.0.1"
#define MAX_BUFFER 100 // Maximum number of messages to keep in the buffer

void receiveMessages(SOCKET s); // Function to receive messages from the server
void drawUI(); // Function to draw the user interface
int getTerminalRows(); // Function to get the number of rows in the terminal
int getTerminalCols(); // Function to get the number of columns in the terminal

std::vector<std::string> chatBuffer; // Buffer to store chat messages

std::mutex chatMutex; // Mutex for thread safety
bool needsRedraw = false; // Flag to indicate if the chat needs to be redrawn
std::string inputBuffer; // Buffer for user input

int scrollOffset = 0; // Offset for scrolling the chat buffer
int terminalCols = 80; // Number of columns in the terminal
int terminalRows = 25; // Number of rows in the terminal
int inputLines = 1; // Number of lines in the input buffer
bool running = true; // Flag to indicate if the program is running
int visibleLines = 0; // Number of visible lines in the chat window

int main() {

    std::cout << "Enter IP address of server (default: " << SERVER << "): ";
    std::string serverInput;
    std::getline(std::cin, serverInput); // Get the server IP address from user input
    const char* serverAddress = serverInput.empty() ? SERVER : serverInput.c_str(); // Use default if empty

    // Initialize Winsock
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    // Create a socket
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(serverAddress);

    // Prompt for the user's name
    std::cout << "Enter your name: ";
    std::string name;
    std::getline(std::cin, name); // Get the user's name

    std::cout << "Connecting to chat server...\n";

    // Connect to the server
    if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    // Add the initial message to the chat buffer
    chatMutex.lock();
    chatBuffer.push_back("Connected to chat server!"); 
    if (chatBuffer.size() > MAX_BUFFER) {
        chatBuffer.erase(chatBuffer.begin()); // Remove the oldest message if buffer exceeds max size
    }
    chatMutex.unlock();

    needsRedraw = true; // Indicate that the chat needs to be redrawn

    send(s, name.c_str(), name.size(), 0); // Send the name to the server

    // Create a thread to receive messages from the server
    std::thread receiver(receiveMessages, s);

    drawUI();
    while(running){
        if(_kbhit()){
            int ch = _getch(); // Get the character input
            if (ch == 224) {
                ch = _getch();
                if (ch == 72) {
                    if (visibleLines - inputLines < (int)chatBuffer.size() - scrollOffset - 1) {
                        scrollOffset = (std::min)(scrollOffset + 1, (int)chatBuffer.size());
                    }
                }
                else if (ch == 80) {
                    scrollOffset = (std::max)(0, scrollOffset - 1);
                }
            } 
            else if(ch == 8) { // Backspace key
                if (!inputBuffer.empty()) {
                    inputBuffer.pop_back();
                }
            }
            else if(ch == 13) { // Enter key
                if (inputBuffer == "/exit") break;
                send(s, inputBuffer.c_str(), inputBuffer.size(), 0);
                inputBuffer.clear();
            }
            else if (isprint(ch)) {
                inputBuffer += (char)ch;
            }
            needsRedraw = true; // Set the redraw flag
        }
        if (needsRedraw) {
            drawUI(); // Redraw the UI if needed
            needsRedraw = false; // Reset the redraw flag
        }
    }

    running = false; // Set the running flag to false to stop the receiver thread
    closesocket(s);
    receiver.join();
    WSACleanup();
    return 0;
}

// Function to receive messages from the server
void receiveMessages(SOCKET s) {
    char buffer[1024];
    while (running) {
        int bytes = recv(s, buffer, sizeof(buffer) - 1, 0); // Receive message from server
        if (bytes <= 0) {
            chatMutex.lock();
            chatBuffer.push_back("Disconnected from server");
            chatMutex.unlock();
            break;
        }
        buffer[bytes] = '\0';
        chatMutex.lock();
        chatBuffer.push_back(buffer);
        if (chatBuffer.size() > MAX_BUFFER) chatBuffer.erase(chatBuffer.begin());
        chatMutex.unlock();
        needsRedraw = true; // Set the redraw flag
    }
}

void drawUI(){
    std::stringstream screen;
    screen << "\033[2J\033[H"; // Clear the screen and move cursor to home position

    terminalRows = getTerminalRows(); // Get the number of rows in the terminal
    terminalCols = getTerminalCols(); // Get the number of columns in the terminal
    if (terminalRows < 5) terminalRows = 5; // Ensure minimum rows for input
    if (terminalCols < 10) terminalCols = 10; // Ensure minimum columns for input

    visibleLines = terminalRows - inputLines - 1; // Calculate the number of visible lines in the chat window

    chatMutex.lock();
    // Calculate the starting index for displaying chat messages
    int start = (std::max)(0, static_cast<int>(chatBuffer.size()) - visibleLines - scrollOffset); 
    for (int i = start; i < (int)chatBuffer.size() - scrollOffset; ++i) {
        screen << chatBuffer[i] << "\n"; // Display the chat messages
    }
    if(int(chatBuffer.size()) < visibleLines) {
        int remainingLines = visibleLines - (int)chatBuffer.size() + scrollOffset;
        for (int i = 0; i < remainingLines; ++i) {
            screen << "\n"; // Fill the remaining lines with empty lines
        }
    }
    chatMutex.unlock();

    // Display the input buffer
    screen << std::string(terminalCols, '-') << "\n";
    screen << "> " << inputBuffer << std::flush;

    std::cout << screen.str(); // Output the screen buffer to the console
}

int getTerminalRows() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int rows = 25; // fallback
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    return rows;
}

int getTerminalCols() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int cols = 80; // fallback
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return cols;
}