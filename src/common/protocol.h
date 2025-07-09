#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>

// Struct to hold a parsed protocol message
struct ProtocolMessage {
    std::string command;
    std::vector<std::string> params;
};

// Builds a protocol message from command and parameters
std::string buildMessage(const std::string& command, const std::vector<std::string>& params);

// Parses a protocol message into command and parameters
ProtocolMessage parseMessage(const std::string& message);

#endif // PROTOCOL_H
