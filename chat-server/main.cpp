#include <iostream>
#include <string>
#include "server.h"

int main(int argc, char* argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
    
    int port = 8080; // Default port
    
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    std::cout << "Starting simple chat server on port " << port << "..." << std::endl;
    
    ChatServer server(port);
    server.start();
    
    std::cout << "Server running. Press Enter to stop." << std::endl;
    std::cin.get();
    
    server.stop();
    
    // Cleanup Winsock
    WSACleanup();
    
    return 0;
} 