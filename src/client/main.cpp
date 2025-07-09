#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include "../common/protocol.h"
#include <algorithm>
#include <fstream>
#include <ctime>

// Debug logging
std::ofstream debugLog;

void initDebugLog() {
    debugLog.open("client_debug.log", std::ios::app);
    if (debugLog.is_open()) {
        time_t now = time(0);
        char* dt = ctime(&now);
        debugLog << "\n=== Client Debug Log Started: " << dt;
        debugLog.flush();
    }
}

void debugLogMsg(const std::string& msg) {
    if (debugLog.is_open()) {
        debugLog << "[DEBUG] " << msg << std::endl;
        debugLog.flush();
    }
}

void closeDebugLog() {
    if (debugLog.is_open()) {
        debugLog.close();
    }
}

void clearScreen() {
    // Use 'clear' for Linux compatibility
    system("clear");
}

void printHeader() {
    std::cout << "==========================================\n";
    std::cout << "        MULTIPLAYER QUIZ GAME\n";
    std::cout << "==========================================\n\n";
}

void printMenu(const std::vector<std::string>& options, const std::string& title = "") {
    if (!title.empty()) {
        std::cout << title << "\n";
        std::cout << std::string(title.length(), '-') << "\n";
    }
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << (i + 1) << ". " << options[i] << "\n";
    }
    std::cout << "\nEnter your choice: ";
}

int getChoice(int maxOptions) {
    int choice;
    while (!(std::cin >> choice) || choice < 1 || choice > maxOptions) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid choice. Please enter a number between 1 and " << maxOptions << ": ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

std::string getInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

void printResponse(const ProtocolMessage& parsed) {
    std::cout << "\nServer Response: " << parsed.command;
    for (const auto& param : parsed.params) {
        std::cout << " [" << param << "]";
    }
    std::cout << "\n";
}

void printSuccess(const std::string& message) {
    std::cout << "\n✓ " << message << "\n";
}

void printError(const std::string& message) {
    std::cout << "\n✗ " << message << "\n";
}

void waitForEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

// Helper to send protocol message with newline if missing
void sendWithNewline(int sock, const std::string& msg) {
    std::string out = msg;
    if (out.empty() || out.back() != '\n') out += '\n';
    send(sock, out.c_str(), static_cast<int>(out.size()), 0);
}

// Helper: handle a game session (wait for questions, display, prompt for answer, submit, repeat)
void playGameSession(int sock, const std::string& username, const std::string& currentRoomId, const std::string* initialQuestionRaw = nullptr) {
    char buffer[1024];
    int recvLen;
    bool first = true;
    std::string leftover;
    while (true) {
        if (first && initialQuestionRaw) {
            debugLogMsg("Using initialQuestionRaw");
            leftover += *initialQuestionRaw;
            // Ensure a newline at the end for correct splitting
            if (leftover.empty() || leftover.back() != '\n') leftover += '\n';
            debugLogMsg("Initial leftover buffer: '" + leftover + "'");
            first = false;
        } else {
            // Robust socket reading: read all available data
            do {
                recvLen = recv(sock, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
                if (recvLen == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
                    break; // No more data for now
                }
                if (recvLen <= 0) {
                    if (recvLen == 0) debugLogMsg("Server closed connection.");
                    return;
                }
                debugLogMsg("Received " + std::to_string(recvLen) + " bytes from server.");
                std::string rawBytes = "Raw bytes received: ";
                for (int i = 0; i < recvLen; ++i) {
                    char hex[4];
                    sprintf(hex, "%02X ", (unsigned char)buffer[i]);
                    rawBytes += hex;
                }
                debugLogMsg(rawBytes);
                debugLogMsg("Raw buffer as string: '" + std::string(buffer, recvLen) + "'");
                buffer[recvLen] = '\0';
                leftover += buffer;
                debugLogMsg("Buffer after append: '" + leftover + "'");
            } while (true);
        }
        // Always process all complete messages in leftover before next recv
        size_t pos;
        while ((pos = leftover.find('\n')) != std::string::npos) {
            std::string message = leftover.substr(0, pos);
            leftover = leftover.substr(pos + 1);
            if (message.empty()) continue; // skip empty lines
            debugLogMsg("Processing message: '" + message + "'");
            ProtocolMessage parsed = parseMessage(message);
            debugLogMsg("Parsed command: '" + parsed.command + "' with " + std::to_string(parsed.params.size()) + " params.");
            bool gameFinished = false;
            for (const auto& param : parsed.params) {
                if (param.find("GAME_FINISHED") != std::string::npos) {
                    gameFinished = true;
                    break;
                }
            }
            if (parsed.command == "QUESTION" && !parsed.params.empty()) {
                ProtocolMessage qmsg = parseMessage(parsed.params[0]);
                if (qmsg.command == "ERROR") {
                    printError(qmsg.params.empty() ? "Game error" : qmsg.params[0]);
                    return;
                }
                if (qmsg.params.size() < 4) {
                    std::cout << "\n[Warning] Malformed question message from server.\n";
                    std::cout << "Raw: " << parsed.params[0] << "\n";
                    return;
                }
                std::cout << "\n[" << qmsg.params[0] << "] (Time left: " << qmsg.params[2] << "s)\n";
                std::cout << qmsg.params[1] << "\n";
                for (size_t i = 3; i < qmsg.params.size(); ++i) {
                    std::cout << qmsg.params[i] << "\n";
                }
                std::string ans = getInput("Enter your answer (option number): ");
                std::string submitMsg = buildMessage("SUBMIT_ANSWER", {username, currentRoomId, ans});
                sendWithNewline(sock, submitMsg);
            } else if (parsed.command == "GAME_RESPONSE" && !parsed.params.empty() && parsed.params[0] == "QUESTION") {
                if (parsed.params.size() < 5) {
                    std::cout << "\n[Warning] Malformed question message from server.\n";
                    std::cout << "Raw: ";
                    for (const auto& p : parsed.params) std::cout << "[" << p << "] ";
                    std::cout << "\n";
                    return;
                }
                std::cout << "\n[" << parsed.params[1] << "] (Time left: " << parsed.params[3] << "s)\n";
                std::cout << parsed.params[2] << "\n";
                for (size_t i = 4; i < parsed.params.size(); ++i) {
                    std::cout << parsed.params[i] << "\n";
                }
                std::string ans = getInput("Enter your answer (option number): ");
                std::string submitMsg = buildMessage("SUBMIT_ANSWER", {username, currentRoomId, ans});
                sendWithNewline(sock, submitMsg);
            } else if (parsed.command == "GAME_RESPONSE" && !parsed.params.empty() && parsed.params[0].rfind("QUESTION|", 0) == 0) {
                ProtocolMessage qmsg = parseMessage(parsed.params[0]);
                if (qmsg.command == "QUESTION") {
                    if (qmsg.params.size() < 4) {
                        std::cout << "\n[Warning] Malformed question message from server.\n";
                        std::cout << "Raw: " << parsed.params[0] << "\n";
                        return;
                    }
                    std::cout << "\n[" << qmsg.params[0] << "] (Time left: " << qmsg.params[2] << "s)\n";
                    std::cout << qmsg.params[1] << "\n";
                    for (size_t i = 3; i < qmsg.params.size(); ++i) {
                        std::cout << qmsg.params[i] << "\n";
                    }
                    std::string ans = getInput("Enter your answer (option number): ");
                    std::string submitMsg = buildMessage("SUBMIT_ANSWER", {username, currentRoomId, ans});
                    sendWithNewline(sock, submitMsg);
                } else if (qmsg.command == "ERROR") {
                    printError(qmsg.params.empty() ? "Game error" : qmsg.params[0]);
                    return;
                }
            } else if (parsed.command == "GAME_RESPONSE" && !parsed.params.empty()) {
                // Handle ANSWER_RESULT
                if (parsed.params[0] == "ANSWER_RESULT") {
                    // Format: ANSWER_RESULT|CORRECT|<correct_answer>|<score>[|GAME_FINISHED]
                    bool correct = (parsed.params.size() > 1 && parsed.params[1] == "CORRECT");
                    std::string correctStr = (parsed.params.size() > 2) ? parsed.params[2] : "?";
                    std::string scoreStr = (parsed.params.size() > 3) ? parsed.params[3] : "?";
                    if (correct) {
                        std::cout << "\nCorrect! The answer was: " << correctStr << ". Your score: " << scoreStr << "\n";
                    } else {
                        std::cout << "\nIncorrect. The correct answer was: " << correctStr << ". Your score: " << scoreStr << "\n";
                    }
                    if (gameFinished) {
                        std::cout << "\nGame over!\n";
                        return;
                    }
                    continue;
                }
                // Handle ERROR|No more questions|GAME_FINISHED
                if (parsed.params[0] == "ERROR" && gameFinished) {
                    std::cout << "\nGame over! No more questions.\n";
                    return;
                }
                // Default: print first param
                std::cout << "\n" << parsed.params[0] << "\n";
                if (gameFinished) {
                    std::cout << "Game finished!\n";
                    return;
                }
            } else if (parsed.command == "LEADERBOARD") {
                std::cout << "\n" << parsed.params[0] << "\n";
                return;
            } else if (parsed.command == "ERROR") {
                printError(parsed.params.empty() ? "Game error" : parsed.params[0]);
                if (gameFinished) {
                    std::cout << "Game finished!\n";
                    return;
                }
                return;
            } else {
                debugLogMsg("Unhandled command: '" + parsed.command + "'");
            }
            if (gameFinished) {
                std::cout << "Game finished!\n";
                return;
            }
        }
    }
    waitForEnter();
}

// Helper to trim whitespace from both ends of a string
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    if (start == std::string::npos || end == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// Helper to check for whitespace or newlines in a string
bool containsWhitespaceOrNewline(const std::string& s) {
    return s.find_first_of(" \t\n\r") != std::string::npos;
}

int main() {
    // Initialize debug logging
    initDebugLog();
    
    int sock = -1;
    struct sockaddr_in serverAddr;
    const int PORT = 8080;
    const char* SERVER_IP = "127.0.0.1";
    char buffer[1024];
    int recvLen;
    std::string username;
    std::string currentRoomId;
    bool loggedIn = false;

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    // 2. Setup server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 3. Connect to server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Connection to server failed." << std::endl;
        close(sock);
        return 1;
    }

    // Main application loop
    while (true) {
        clearScreen();
        printHeader();
        
        if (!loggedIn) {
            // Authentication menu
            std::vector<std::string> authOptions = {"Register", "Login", "Quit"};
            printMenu(authOptions, "AUTHENTICATION");
            
            int choice = getChoice(authOptions.size());
            
            if (choice == 1) { // Register
                clearScreen();
                printHeader();
                std::cout << "REGISTRATION\n";
                std::cout << "============\n\n";
                while (true) {
                    username = trim(getInput("Enter username: "));
                    std::string password = trim(getInput("Enter password: "));
                    if (username.empty() || password.empty()) {
                        printError("Username and password cannot be empty.");
                        continue;
                    }
                    if (containsWhitespaceOrNewline(username) || containsWhitespaceOrNewline(password)) {
                        printError("Username and password must not contain spaces or newlines. Please try again.");
                        continue;
                    }
                    debugLogMsg("Registering with username: '" + username + "' password: '" + password + "'");
                    // Print raw bytes of username
                    std::string usernameBytes = "Username raw bytes: ";
                    for (size_t i = 0; i < username.size(); ++i) {
                        char hex[4];
                        sprintf(hex, "%02X ", (unsigned char)username[i]);
                        usernameBytes += hex;
                    }
                    debugLogMsg(usernameBytes);
                    // Print raw bytes of password
                    std::string passwordBytes = "Password raw bytes: ";
                    for (size_t i = 0; i < password.size(); ++i) {
                        char hex[4];
                        sprintf(hex, "%02X ", (unsigned char)password[i]);
                        passwordBytes += hex;
                    }
                    debugLogMsg(passwordBytes);
                    std::string registerMsg = buildMessage("REGISTER", {username, password});
                    sendWithNewline(sock, registerMsg);
                    recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                    if (recvLen > 0) {
                        buffer[recvLen] = '\0';
                        ProtocolMessage parsed = parseMessage(buffer);
                        printResponse(parsed);
                        if (parsed.command == "OK") {
                            printSuccess("Registration successful! Please login.");
                            break;
                        } else if (parsed.command == "ERROR" && !parsed.params.empty()) {
                            printError(parsed.params[0]);
                        } else {
                            printError("Registration failed.");
                        }
                    }
                    waitForEnter();
                }
                waitForEnter();
                
            } else if (choice == 2) { // Login
                clearScreen();
                printHeader();
                std::cout << "LOGIN\n";
                std::cout << "=====\n\n";
                while (true) {
                    username = trim(getInput("Enter username: "));
                    std::string password = trim(getInput("Enter password: "));
                    if (username.empty() || password.empty()) {
                        printError("Username and password cannot be empty.");
                        continue;
                    }
                    if (containsWhitespaceOrNewline(username) || containsWhitespaceOrNewline(password)) {
                        printError("Username and password must not contain spaces or newlines. Please try again.");
                        continue;
                    }
                    debugLogMsg("Logging in with username: '" + username + "' password: '" + password + "'");
                    std::string loginMsg = buildMessage("LOGIN", {username, password});
                    sendWithNewline(sock, loginMsg);
                    recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                    if (recvLen > 0) {
                        buffer[recvLen] = '\0';
                        ProtocolMessage parsed = parseMessage(buffer);
                        printResponse(parsed);
                        if (parsed.command == "OK") {
                            loggedIn = true;
                            printSuccess("Login successful!");
                            break;
                        } else if (parsed.command == "ERROR" && !parsed.params.empty()) {
                            printError(parsed.params[0]);
                        } else {
                            printError("Login failed.");
                        }
                    }
                    waitForEnter();
                }
                waitForEnter();
                
            } else if (choice == 3) { // Quit
                std::string quitMsg = buildMessage("QUIT", {});
                sendWithNewline(sock, quitMsg);
                close(sock);
                std::cout << "Goodbye!\n";
                return 0;
            }
            
        } else {
            // Main menu (after login)
            std::cout << "Welcome, " << username << "!\n";
            if (!currentRoomId.empty()) {
                std::cout << "Current Room: " << currentRoomId << "\n";
            }
            std::cout << "\n";
            
            std::vector<std::string> mainOptions = {"Room Management", "Game Actions", "Logout", "Quit"};
            printMenu(mainOptions, "MAIN MENU");
            
            int choice = getChoice(mainOptions.size());
            
            if (choice == 1) { // Room Management
                while (true) {
                    clearScreen();
                    printHeader();
                    std::cout << "ROOM MANAGEMENT\n";
                    std::cout << "================\n\n";
                    
                    if (!currentRoomId.empty()) {
                        std::cout << "Current Room: " << currentRoomId << "\n\n";
                    }
                    
                    std::vector<std::string> roomOptions = {"Create Room", "Browse Rooms", "Join Room", "Back to Main Menu"};
                    printMenu(roomOptions);
                    
                    int roomChoice = getChoice(roomOptions.size());
                    
                    if (roomChoice == 1) { // Create Room
                        std::string roomName = getInput("Enter room name: ");
                        std::string createRoomMsg = buildMessage("CREATE_ROOM", {username, roomName});
                        sendWithNewline(sock, createRoomMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                            if (parsed.command == "OK" && parsed.params.size() >= 2) {
                                currentRoomId = parsed.params[1];
                                printSuccess("Room created and joined successfully!");
                            } else {
                                printError("Failed to create room.");
                            }
                        }
                        waitForEnter();
                        
                    } else if (roomChoice == 2) { // Browse Rooms
                        std::string browseMsg = buildMessage("BROWSE_ROOMS", {});
                        sendWithNewline(sock, browseMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            std::cout << "\nAvailable Rooms:\n";
                            std::cout << "================\n";
                            for (size_t i = 0; i + 1 < parsed.params.size(); i += 2) {
                                std::cout << "ID: " << parsed.params[i] << " | Name: " << parsed.params[i+1] << "\n";
                            }
                            if (parsed.params.empty()) {
                                std::cout << "No rooms available.\n";
                            }
                        }
                        waitForEnter();
                        
                    } else if (roomChoice == 3) { // Join Room
                        std::string roomIdStr = getInput("Enter room ID to join: ");
                        std::string joinMsg = buildMessage("JOIN_ROOM", {username, roomIdStr});
                        sendWithNewline(sock, joinMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                            if (parsed.command == "OK") {
                                currentRoomId = roomIdStr;
                                printSuccess("Joined room successfully!");
                                std::cout << "Waiting for the game to start...\n";
                                // Wait for a QUESTION message before starting playGameSession
                                while (true) {
                                    recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                                    if (recvLen > 0) {
                                        buffer[recvLen] = '\0';
                                        ProtocolMessage waitMsg = parseMessage(buffer);
                                        if (waitMsg.command == "QUESTION") {
                                            // Put the QUESTION message back into the buffer for playGameSession
                                            std::string questionRaw = buffer;
                                            playGameSession(sock, username, currentRoomId, &questionRaw);
                                            break;
                                        } else if (waitMsg.command == "GAME_RESPONSE" && !waitMsg.params.empty() && waitMsg.params[0] == "QUESTION") {
                                            // Handle GAME_RESPONSE containing a QUESTION payload
                                            std::string questionRaw = buffer;
                                            playGameSession(sock, username, currentRoomId, &questionRaw);
                                            break;
                                        } else if (waitMsg.command == "ERROR") {
                                            printError(waitMsg.params.empty() ? "Game error" : waitMsg.params[0]);
                                            break;
                                        } else {
                                            // Print any other server messages while waiting
                                            printResponse(waitMsg);
                                        }
                                    }
                                }
                            } else {
                                printError("Failed to join room.");
                            }
                        }
                        waitForEnter();
                        
                    } else if (roomChoice == 4) { // Back to Main Menu
                        break;
                    }
                }
                
            } else if (choice == 2) { // Game Actions
                if (currentRoomId.empty()) {
                    printError("You must join or create a room first.");
                    waitForEnter();
                    continue;
                }
                
                while (true) {
                    clearScreen();
                    printHeader();
                    std::cout << "GAME ACTIONS\n";
                    std::cout << "============\n\n";
                    std::cout << "Room: " << currentRoomId << "\n\n";
                    
                    std::vector<std::string> gameOptions = {"Start Game", "Get Current Question", "Submit Answer", "Get Game Info", "Get Leaderboard", "End Game", "Back to Main Menu"};
                    printMenu(gameOptions);
                    
                    int gameChoice = getChoice(gameOptions.size());
                    
                    if (gameChoice == 1) { // Start Game
                        std::string qCount = getInput("How many questions? (default 5): ");
                        if (qCount.empty()) qCount = "5";
                        std::string startMsg = buildMessage("START_GAME", {username, currentRoomId, qCount});
                        sendWithNewline(sock, startMsg);
                        char buffer[1024];
                        int recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        std::string leftover;
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            leftover = buffer;
                        }
                        // Enter playGameSession for the entire game flow
                        playGameSession(sock, username, currentRoomId, leftover.empty() ? nullptr : &leftover);
                        waitForEnter();
                        
                    } else if (gameChoice == 2) { // Get Current Question
                        std::string getQMsg = buildMessage("GET_CURRENT_QUESTION", {username, currentRoomId});
                        sendWithNewline(sock, getQMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                            
                            // Immediately ask for answer if question was received
                            if (parsed.command == "GAME_RESPONSE" && !parsed.params.empty()) {
                                std::cout << "\n";
                                std::string ans = getInput("Enter your answer (option number): ");
                                std::string submitMsg = buildMessage("SUBMIT_ANSWER", {username, currentRoomId, ans});
                                sendWithNewline(sock, submitMsg);
                                
                                recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                                if (recvLen > 0) {
                                    buffer[recvLen] = '\0';
                                    ProtocolMessage answerParsed = parseMessage(buffer);
                                    printResponse(answerParsed);
                                }
                            }
                        }
                        waitForEnter();
                        
                    } else if (gameChoice == 3) { // Submit Answer
                        std::string ans = getInput("Enter your answer (option number): ");
                        std::string submitMsg = buildMessage("SUBMIT_ANSWER", {username, currentRoomId, ans});
                        sendWithNewline(sock, submitMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                        }
                        waitForEnter();
                        
                    } else if (gameChoice == 4) { // Get Game Info
                        std::string infoMsg = buildMessage("GET_GAME_INFO", {username, currentRoomId});
                        sendWithNewline(sock, infoMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                        }
                        waitForEnter();
                        
                    } else if (gameChoice == 5) { // Get Leaderboard
                        std::string lbMsg = buildMessage("GET_LEADERBOARD", {username, currentRoomId});
                        sendWithNewline(sock, lbMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                        }
                        waitForEnter();
                        
                    } else if (gameChoice == 6) { // End Game
                        std::string endMsg = buildMessage("END_GAME", {username, currentRoomId});
                        sendWithNewline(sock, endMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                        }
                        waitForEnter();
                        
                    } else if (gameChoice == 7) { // Back to Main Menu
                        break;
                    }
                }
                
            } else if (choice == 3) { // Logout
                loggedIn = false;
                username.clear();
                currentRoomId.clear();
                printSuccess("Logged out successfully.");
                waitForEnter();
                
            } else if (choice == 4) { // Quit
                std::string quitMsg = buildMessage("QUIT", {});
                sendWithNewline(sock, quitMsg);
                close(sock);
                std::cout << "Goodbye!\n";
                return 0;
            }
        }
    }
    close(sock);
    closeDebugLog();
    return 0;
} 