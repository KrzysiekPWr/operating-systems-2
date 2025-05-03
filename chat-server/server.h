#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

// Forward declarations
class Client;
class ChatRoom;

// Simple message structure
struct Message {
    std::string sender;
    std::string content;
    std::string timestamp;
    
    Message(const std::string& from, const std::string& text);
    std::string formatMessage() const;
};

// Client handler class
class Client {
private:
    SOCKET socket_fd;
    std::string username;
    std::thread client_thread;
    std::mutex write_mutex;
    bool is_running;
    
public:
    Client(SOCKET socket, const std::string& name);
    ~Client();
    
    void start(std::function<void(const std::string&, const std::string&)> message_handler);
    void stop();
    bool isRunning() const;
    void sendMessage(const Message& msg);
    std::string getUsername() const;
};

// Chat room class
class ChatRoom {
private:
    std::string name;
    std::vector<Message> message_history;
    mutable std::mutex history_mutex;
    
public:
    ChatRoom(const std::string& room_name);
    void addMessage(const Message& msg);
    std::vector<Message> getHistory() const;
    std::string getName() const;
};

// Main ChatServer class
class ChatServer {
private:
    SOCKET server_socket;
    int port;
    std::atomic<bool> running;
    std::thread accept_thread;
    
    std::map<std::string, std::shared_ptr<Client>> clients;
    std::mutex clients_mutex;
    
    std::shared_ptr<ChatRoom> chat_room;
    
    void acceptClients();
    void handleClientMessage(const std::string& sender, const std::string& message);
    void broadcastMessage(const Message& msg);
    
public:
    ChatServer(int server_port);
    ~ChatServer();
    
    void start();
    void stop();
    std::string getCurrentTimestamp() const;
}; 