#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

class ChatClient {
private:
    SOCKET client_socket;
    std::string server_ip;
    int server_port;
    std::string username;
    std::thread receive_thread;
    std::atomic<bool> running;
    std::mutex print_mutex;
    
    void receiveMessages() {
        char buffer[1024];
        
        while (running) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_read <= 0) {
                // Server disconnected
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cout << "\nDisconnected from server." << std::endl;
                running = false;
                break;
            }
            
            buffer[bytes_read] = '\0';
            std::string message(buffer);
            
            // Print received message (unless it's just the username prompt)
            std::lock_guard<std::mutex> lock(print_mutex);
            if (message.find("Enter your username:") != std::string::npos) {
                std::cout << "Server is asking for username." << std::endl;
            } else {
                std::cout << buffer << std::flush;
            }
        }
    }
    
public:
    ChatClient(const std::string& ip, int port)
        : server_ip(ip), server_port(port), running(false) {
    }
    
    ~ChatClient() {
        disconnect();
    }
    
    std::string getUsername() const {
        return username;
    }
    
    bool connect() {
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }
        
        // Create socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return false;
        }
        
        // Set up server address
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        
        // Convert IP address from text to binary
        if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address or address not supported" << std::endl;
            closesocket(client_socket);
            WSACleanup();
            return false;
        }
        
        // Connect to server
        if (::connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
            closesocket(client_socket);
            WSACleanup();
            return false;
        }
        
        std::cout << "Connected to server " << server_ip << ":" << server_port << std::endl;
        std::cout << "Waiting for username prompt..." << std::endl;
        
        running = true;
        receive_thread = std::thread(&ChatClient::receiveMessages, this);
        
        return true;
    }
    
    void start() {
        std::string input;
        
        // Wait for username prompt
        std::cout << "Waiting for server prompt..." << std::endl;
        // The receiveMessages thread will show the "Enter your username:" prompt
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // First input will be the username
        std::cout << "Enter your username: ";
        std::getline(std::cin, input);
        username = input;
        
        // Send username to server
        input += "\n";
        send(client_socket, input.c_str(), (int)input.length(), 0);
        
        // Now start the normal message loop
        while (running) {
            std::getline(std::cin, input);
            
            // Check if client is still running
            if (!running) {
                break;
            }
            
            // Exit command
            if (input == "/exit") {
                break;
            }
            
            // Send message to server
            input += "\n";
            send(client_socket, input.c_str(), (int)input.length(), 0);
        }
        
        disconnect();
    }
    
    void disconnect() {
        running = false;
        
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        
        closesocket(client_socket);
        WSACleanup();
    }
};

void printHelp() {
    std::cout << "Chat Client Commands:" << std::endl;
    std::cout << "  /help - Show this help" << std::endl;
    std::cout << "  /exit - Exit the client" << std::endl;
    std::cout << "Any other text is sent to all connected users." << std::endl;
}

int main(int argc, char* argv[]) {
    std::string server_ip = "127.0.0.1"; // Default to localhost
    int server_port = 8080; // Default port
    
    // Parse command line arguments
    if (argc > 1) {
        server_ip = argv[1];
    }
    
    if (argc > 2) {
        server_port = std::stoi(argv[2]);
    }
    
    std::cout << "Simple Chat Client" << std::endl;
    std::cout << "Connecting to " << server_ip << ":" << server_port << std::endl;
    
    ChatClient client(server_ip, server_port);
    
    if (!client.connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    printHelp();
    client.start();
    
    std::cout << "Client disconnected" << std::endl;
    return 0;
} 