#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>

class Message {
private:
    std::string sender;
    std::string content;
    std::time_t timestamp;

public:
    Message(const std::string& sender, const std::string& content);
    
    std::string getSender() const;
    std::string getContent() const;
    std::time_t getTimestamp() const;
    
    std::string getFormattedMessage() const;
    std::string getFormattedTimestamp() const;
};

#endif // MESSAGE_H 