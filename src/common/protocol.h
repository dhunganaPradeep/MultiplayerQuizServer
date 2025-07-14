#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <vector>


struct ProtocolMessage {
    std::string command;
    std::vector<std::string> params;
};


std::string buildMessage(const std::string& command, const std::vector<std::string>& params);


ProtocolMessage parseMessage(const std::string& message);

#endif 
