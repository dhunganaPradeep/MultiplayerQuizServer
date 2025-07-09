#include "protocol.h"
#include <sstream>

// Builds a protocol message from command and parameters
std::string buildMessage(const std::string& command, const std::vector<std::string>& params) {
    std::ostringstream oss;
    oss << command;
    for (const auto& param : params) {
        oss << '|' << param;
    }
    return oss.str();
}

// Parses a protocol message into command and parameters
ProtocolMessage parseMessage(const std::string& message) {
    ProtocolMessage result;
    std::istringstream iss(message);
    std::string token;
    bool first = true;
    while (std::getline(iss, token, '|')) {
        if (first) {
            result.command = token;
            first = false;
        } else {
            result.params.push_back(token);
        }
    }
    return result;
}
