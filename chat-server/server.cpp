#include "server.h"
#include <ctime>
#include <iomanip>

// Message implementation
Message::Message(const std::string& from, const std::string& text) 
    : sender(from), content(text) {
    // Set timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm timeinfo;
    localtime_s(&timeinfo, &time);
    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%H:%M:%S");
    timestamp = ss.str();
}

std::string Message::formatMessage() const {
    return "[" + timestamp + "] " + sender + ": " + content;
}

// Client implementation
Client::Client(SOCKET socket, const std::string& name) 
    : socket_fd(socket), username(name), is_running(true) {
}

Client::~Client() {
    stop();
    if (client_thread.joinable()) {
        client_thread.join();
    }
    closesocket(socket_fd);
}

void Client::start(std::function<void(const std::string&, const std::string&)> message_handler) {
    client_thread = std::thread([this, message_handler]() {
        char buffer[1024];
        
        // Send welcome message
        std::string welcome = "Welcome to the chat server, " + username + "!\n";
        send(socket_fd, welcome.c_str(), (int)welcome.length(), 0);
        
        while (is_running) {
            // Clear buffer
            memset(buffer, 0, sizeof(buffer));
            
            // Check for incoming messages
            fd_set readfds;
            struct timeval tv;
            FD_ZERO(&readfds);
            FD_SET(socket_fd, &readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100ms
            
            if (select(0, &readfds, NULL, NULL, &tv) > 0) {
                int bytes_read = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
                
                if (bytes_read <= 0) {
                    // Client disconnected
                    is_running = false;
                    break;
                }
                
                buffer[bytes_read] = '\0';
                
                // Process command or message
                std::string message(buffer);
                
                // Remove trailing newline
                size_t pos = message.find_last_not_of("\r\n");
                if (pos != std::string::npos) {
                    message.erase(pos + 1);
                }
                
                if (message.substr(0, 5) == "/help") {
                    std::string help = "Available commands:\n"
                                     "/help - Show this help\n"
                                     "/exit - Exit the chat\n";
                    send(socket_fd, help.c_str(), (int)help.length(), 0);
                } 
                else {
                    // Broadcast to all users
                    if (!message.empty()) {
                        message_handler(username, message);
                    }
                }
            }
            
            // Add a small sleep to avoid 100% CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}

void Client::stop() {
    is_running = false;
}

bool Client::isRunning() const {
    return is_running;
}

void Client::sendMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(write_mutex);
    std::string formatted = msg.formatMessage() + "\n";
    send(socket_fd, formatted.c_str(), (int)formatted.length(), 0);
}

std::string Client::getUsername() const {
    return username;
}

// ChatRoom implementation
ChatRoom::ChatRoom(const std::string& room_name) : name(room_name) {
}

void ChatRoom::addMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(history_mutex);
    message_history.push_back(msg);
}

std::vector<Message> ChatRoom::getHistory() const {
    std::lock_guard<std::mutex> lock(history_mutex);
    return message_history;
}

std::string ChatRoom::getName() const {
    return name;
}

// ChatServer implementation
ChatServer::ChatServer(int server_port) : port(server_port), running(false) {
}

ChatServer::~ChatServer() {
    stop();
}

void ChatServer::start() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }
    
    // Listen for connections
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Error listening: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }
    
    // Create the chat room
    chat_room = std::make_shared<ChatRoom>("Chat Room");
    
    std::cout << "Server is running on port " << port << std::endl;
    
    running = true;
    accept_thread = std::thread(&ChatServer::acceptClients, this);
}

void ChatServer::stop() {
    running = false;
    
    if (accept_thread.joinable()) {
        accept_thread.join();
    }
    
    // Close all client connections
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto& client_pair : clients) {
        client_pair.second->stop();
    }
    clients.clear();
    
    closesocket(server_socket);
    WSACleanup();
    std::cout << "Server stopped" << std::endl;
}

void ChatServer::acceptClients() {
    while (running) {
        // Set up select for accepting connections with timeout
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        tv.tv_sec = 1; // 1 second timeout to check running flag
        tv.tv_usec = 0;
        
        if (select(0, &readfds, NULL, NULL, &tv) <= 0) {
            continue; // Timeout or error, check running flag
        }
        
        // Accept connection
        struct sockaddr_in client_addr;
        int client_size = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_size);
        
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection: " << WSAGetLastError() << std::endl;
            continue;
        }
        
        // Get client username
        char buffer[1024];
        std::string prompt = "Enter your username: ";
        std::cout << "Sending username prompt to client..." << std::endl;
        send(client_socket, prompt.c_str(), (int)prompt.length(), 0);
        
        std::cout << "Waiting for client username..." << std::endl;
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            std::cerr << "Failed to receive username from client" << std::endl;
            closesocket(client_socket);
            continue;
        }
        
        buffer[bytes_read] = '\0';
        std::string username(buffer);
        
        // Remove trailing newline
        size_t pos = username.find_last_not_of("\r\n");
        if (pos != std::string::npos) {
            username.erase(pos + 1);
        }
        
        std::cout << "Received username: '" << username << "'" << std::endl;
        
        // Check if username already exists
        std::unique_lock<std::mutex> lock(clients_mutex);
        if (clients.find(username) != clients.end()) {
            lock.unlock();
            std::string error = "Username already taken. Connection closed.\n";
            send(client_socket, error.c_str(), (int)error.length(), 0);
            closesocket(client_socket);
            continue;
        }
        
        // Create new client
        std::shared_ptr<Client> client = std::make_shared<Client>(client_socket, username);
        clients[username] = client;
        lock.unlock();
        
        // Start client thread
        client->start([this](const std::string& sender, const std::string& message) {
            this->handleClientMessage(sender, message);
        });
        
        // Send welcome message to all clients
        Message welcome_msg("Server", username + " has joined the chat");
        
        // Add to chat history
        chat_room->addMessage(welcome_msg);
        
        // Broadcast to all clients
        broadcastMessage(welcome_msg);
        
        // Send chat history to new client
        auto history = chat_room->getHistory();
        for (const auto& msg : history) {
            if (msg.sender != "Server" || msg.content != username + " has joined the chat") {
                client->sendMessage(msg);
            }
        }
        
        std::cout << "New client connected: " << username << std::endl;
    }
}

void ChatServer::handleClientMessage(const std::string& sender, const std::string& message) {
    // Create message
    Message msg(sender, message);
    
    // Add to chat history
    chat_room->addMessage(msg);
    
    // Broadcast to all clients
    broadcastMessage(msg);
}

void ChatServer::broadcastMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    for (auto& client_pair : clients) {
        client_pair.second->sendMessage(msg);
    }
}

std::string ChatServer::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm timeinfo;
    localtime_s(&timeinfo, &time);
    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%H:%M:%S");
    return ss.str();
} 