#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "../common/protocol.h"
#include "authentication.h"
#include "room_manager.h"
#include "question_manager.h"
#include "game_engine.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <ctime>

#include "debug_log.h"

// POSIX socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

struct ClientSession {
    int socket;
    std::string username;
    bool authenticated;
    int currentRoomId;
    
    ClientSession(int s) : socket(s), authenticated(false), currentRoomId(-1) {}
};

// Helper: send a message to a client by username
void sendToClient(const std::string& username, const std::string& message, std::map<int, ClientSession>& clients) {
    std::string msgWithNewline = message + "\n";
    debugLogMsg("Sending to " + username + ": '" + message + "' (with newline)");
    std::string rawBytes = "Raw bytes: ";
    for (unsigned char c : msgWithNewline) {
        char hex[4];
        sprintf(hex, "%02X ", c);
        rawBytes += hex;
    }
    debugLogMsg(rawBytes);
    for (auto& [sock, session] : clients) {
        if (session.username == username) {
            send(sock, msgWithNewline.c_str(), msgWithNewline.size(), 0);
            break;
        }
    }
}
// Helper: broadcast a message to all players in a room
void broadcastToRoom(int /*roomId*/, const std::vector<std::string>& players, const std::string& message, std::map<int, ClientSession>& clients) {
    debugLogMsg("Broadcasting to room: '" + message + "'");
    for (const auto& username : players) {
        sendToClient(username, message, clients);
    }
}

// Function to process commands and return responses
std::string processCommand(const ProtocolMessage& parsed, ClientSession& session, 
                          AuthenticationManager& authManager, RoomManager& roomManager, GameEngine& gameEngine, std::map<int, ClientSession>& clients) {
    std::string response;
    
    if (parsed.command == "REGISTER") {
        if (parsed.params.size() >= 2) {
            bool success = authManager.registerUser(parsed.params[0], parsed.params[1]);
            if (success) {
                response = buildMessage("OK", {SuccessMessages::REGISTRATION_SUCCESS});
            } else {
                response = buildMessage("ERROR", {ErrorMessages::USERNAME_TAKEN});
            }
        } else {
            response = buildMessage("ERROR", {"Invalid registration parameters"});
        }
    } else if (parsed.command == "LOGIN") {
        if (parsed.params.size() >= 2) {
            bool success = authManager.authenticateUser(parsed.params[0], parsed.params[1]);
            if (success) {
                session.username = parsed.params[0];
                session.authenticated = true;
                response = buildMessage("OK", {SuccessMessages::LOGIN_SUCCESS});
            } else {
                response = buildMessage("ERROR", {ErrorMessages::INVALID_CREDENTIALS});
            }
        } else {
            response = buildMessage("ERROR", {"Invalid login parameters"});
        }
    } else if (parsed.command == "CREATE_ROOM") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (parsed.params.size() >= 2) {
            std::string roomName = parsed.params[1];
            int roomId = roomManager.createRoom(roomName, session.username);
            if (roomId > 0) {
                session.currentRoomId = roomId;
                response = buildMessage("OK", {SuccessMessages::ROOM_CREATED, std::to_string(roomId)});
            } else {
                response = buildMessage("ERROR", {"Failed to create room"});
            }
        } else {
            response = buildMessage("ERROR", {"Invalid room creation parameters"});
        }
    } else if (parsed.command == "JOIN_ROOM") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (parsed.params.size() >= 2) {
            int roomId = std::stoi(parsed.params[1]);
            JoinRoomResult joinResult = roomManager.joinRoom(roomId, session.username);
            if (joinResult == JoinRoomResult::SUCCESS) {
                session.currentRoomId = roomId;
                response = buildMessage("OK", {SuccessMessages::ROOM_JOINED});
            } else if (joinResult == JoinRoomResult::ROOM_NOT_FOUND) {
                response = buildMessage("ERROR", {ErrorMessages::ROOM_NOT_FOUND});
            } else if (joinResult == JoinRoomResult::ROOM_FULL) {
                response = buildMessage("ERROR", {ErrorMessages::ROOM_FULL});
            } else if (joinResult == JoinRoomResult::GAME_IN_PROGRESS) {
                response = buildMessage("ERROR", {ErrorMessages::GAME_ALREADY_STARTED});
            } else if (joinResult == JoinRoomResult::USER_ALREADY_IN_ROOM) {
                response = buildMessage("ERROR", {"User is already in a room"});
            } else if (joinResult == JoinRoomResult::USER_ALREADY_IN_THIS_ROOM) {
                response = buildMessage("ERROR", {"User is already in this room"});
            } else {
                response = buildMessage("ERROR", {"Unknown join error"});
            }
        } else {
            response = buildMessage("ERROR", {"Invalid join room parameters"});
        }
    } else if (parsed.command == "BROWSE_ROOMS") {
        std::vector<Room> availableRooms = roomManager.getAvailableRooms();
        std::vector<std::string> roomList;
        for (const auto& room : availableRooms) {
            roomList.push_back(std::to_string(room.getRoomId()));
            roomList.push_back(room.getRoomName());
        }
        response = buildMessage("ROOM_LIST", roomList);
    } else if (parsed.command == "START_GAME") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (session.currentRoomId == -1) {
            response = buildMessage("ERROR", {"Not in a room"});
        } else if (parsed.params.size() >= 2) {
            int questionCount = (parsed.params.size() >= 3) ? std::stoi(parsed.params[2]) : 10;
            std::string result = gameEngine.startGame(session.currentRoomId, session.username, questionCount);
            response = buildMessage("GAME_RESPONSE", {result});
            // Send first question to all players
            auto players = roomManager.getRoomPlayers(session.currentRoomId);
            std::string playersList = "START_GAME: Sending first question to players: ";
            for (const auto& player : players) playersList += player + " ";
            debugLogMsg(playersList);
            for (const auto& player : players) {
                std::string qmsg = gameEngine.getCurrentQuestion(session.currentRoomId, player);
                std::string fullMsg = buildMessage("GAME_RESPONSE", {qmsg});
                debugLogMsg("To player '" + player + "': '" + fullMsg + "'");
                sendToClient(player, fullMsg, clients);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } else {
            response = buildMessage("ERROR", {"Invalid start game parameters"});
        }
    } else if (parsed.command == "END_GAME") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (session.currentRoomId == -1) {
            response = buildMessage("ERROR", {"Not in a room"});
        } else {
            std::string result = gameEngine.endGame(session.currentRoomId, session.username);
            response = buildMessage("GAME_RESPONSE", {result});
        }
    } else if (parsed.command == "GET_CURRENT_QUESTION") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (session.currentRoomId == -1) {
            response = buildMessage("ERROR", {"Not in a room"});
        } else {
            std::string result = gameEngine.getCurrentQuestion(session.currentRoomId, session.username);
            response = buildMessage("GAME_RESPONSE", {result});
        }
    } else if (parsed.command == "SUBMIT_ANSWER") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (session.currentRoomId == -1) {
            response = buildMessage("ERROR", {"Not in a room"});
        } else if (parsed.params.size() >= 3) {
            int answerIndex = std::stoi(parsed.params[2]);
            std::string result = gameEngine.submitAnswer(session.currentRoomId, session.username, answerIndex);
            response = buildMessage("GAME_RESPONSE", {result});
            // Send next question to this player
            std::string qmsg = gameEngine.getCurrentQuestion(session.currentRoomId, session.username);
            sendToClient(session.username, buildMessage("GAME_RESPONSE", {qmsg}), clients);
        } else {
            response = buildMessage("ERROR", {"Invalid submit answer parameters"});
        }
    } else if (parsed.command == "GET_GAME_INFO") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (session.currentRoomId == -1) {
            response = buildMessage("ERROR", {"Not in a room"});
        } else {
            std::string result = gameEngine.getGameInfo(session.currentRoomId, session.username);
            response = buildMessage("GAME_RESPONSE", {result});
        }
    } else if (parsed.command == "GET_LEADERBOARD") {
        if (!session.authenticated) {
            response = buildMessage("ERROR", {"Not authenticated"});
        } else if (session.currentRoomId == -1) {
            response = buildMessage("ERROR", {"Not in a room"});
        } else {
            std::string result = gameEngine.getLeaderboard(session.currentRoomId, session.username);
            response = buildMessage("GAME_RESPONSE", {result});
        }
    } else if (parsed.command == "QUIT") {
        response = buildMessage("OK", {"Goodbye"});
    } else {
        response = buildMessage("ERROR", {"Unknown command"});
    }
    
    return response;
}

int main() {
    // Initialize debug logging
    initDebugLog();
    
    int listenSocket = -1;
    const int PORT = 8080;
    char buffer[1024];
    
    // Initialize managers
    AuthenticationManager authManager;
    RoomManager roomManager;
    QuestionManager questionManager;
    GameEngine gameEngine(roomManager, questionManager);

    // Client management
    std::map<int, ClientSession> clients;
    fd_set master, read_fds;

    // 1. Create socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    // 2. Set socket to non-blocking
    int flags = fcntl(listenSocket, F_GETFL, 0);
    fcntl(listenSocket, F_SETFL, flags | O_NONBLOCK);

    // 3. Bind
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Bind failed." << std::endl;
        close(listenSocket);
        return 1;
    }

    // 4. Listen
    if (listen(listenSocket, SOMAXCONN) == -1) {
        std::cerr << "Listen failed." << std::endl;
        close(listenSocket);
        return 1;
    }
    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    // 5. Initialize select sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(listenSocket, &master);

    // 6. Main server loop
    std::cout << "Server ready for multiple clients..." << std::endl;
    
    while (true) {
        // Copy the master set for select
        read_fds = master;
        
        // Wait for activity on any socket
        int maxfd = listenSocket;
        for (const auto& c : clients) if (c.first > maxfd) maxfd = c.first;
        int activity = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
        if (activity == -1) {
            std::cerr << "Select failed with error: " << errno << std::endl;
            break;
        }

        // Check for new connections
        if (FD_ISSET(listenSocket, &read_fds)) {
            int clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket != -1) {
                int clientFlags = fcntl(clientSocket, F_GETFL, 0);
                fcntl(clientSocket, F_SETFL, clientFlags | O_NONBLOCK);
                
                // Add to master set
                FD_SET(clientSocket, &master);
                
                // Create client session
                clients.emplace(clientSocket, ClientSession(clientSocket));
                
                std::cout << "Client connected. Socket: " << clientSocket << " Total clients: " << clients.size() << std::endl;
            }
        }

        // Check all clients for data
        std::vector<int> toRemove;
        
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            int clientSocket = it->first;
            ClientSession& session = it->second;
            
            if (FD_ISSET(clientSocket, &read_fds)) {
                // Try to receive data
                int recvLen = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                
                if (recvLen > 0) {
                    // Data received
                    buffer[recvLen] = '\0';
                    std::string msg(buffer);
                    
                    // Remove trailing newline before parsing
                    if (!msg.empty() && msg.back() == '\n') {
                        msg.pop_back();
                    }
                    
                    // Parse message
                    ProtocolMessage parsed = parseMessage(msg);
                    debugLogMsg("Received from client " + std::to_string(clientSocket) + " (" + session.username + "): '" + msg + "' Parsed command: '" + parsed.command + "'");
                    
                    // Process command and send response
                    std::string response = processCommand(parsed, session, authManager, roomManager, gameEngine, clients);
                    
                    // Ensure response ends with a newline
                    if (response.empty() || response.back() != '\n') response += '\n';
                    int sendResult = send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
                    if (sendResult == -1) {
                        std::cerr << "Send failed for client " << clientSocket << std::endl;
                        toRemove.push_back(clientSocket);
                    }
                    
                } else if (recvLen == 0) {
                    // Client disconnected
                    std::cout << "Client " << clientSocket << " (" << session.username << ") disconnected." << std::endl;
                    toRemove.push_back(clientSocket);
                } else {
                    // Error or would block
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        std::cerr << "Receive error for client " << clientSocket << ": " << errno << std::endl;
                        toRemove.push_back(clientSocket);
                    }
                }
            }
        }

        // Remove disconnected clients
        for (int clientSocket : toRemove) {
            auto it = clients.find(clientSocket);
            if (it != clients.end()) {
                ClientSession& session = it->second;
                if (session.currentRoomId != -1) {
                    gameEngine.removePlayer(session.currentRoomId, session.username);
                }
            }
            
            FD_CLR(clientSocket, &master);
            close(clientSocket);
            clients.erase(clientSocket);
            
            std::cout << "Client " << clientSocket << " removed. Total clients: " << clients.size() << std::endl;
        }
    }

    // 8. Clean up
    for (auto& pair : clients) {
        close(pair.first);
    }
    close(listenSocket);
    closeDebugLog();
    std::cout << "Server shut down." << std::endl;
    return 0;
} 