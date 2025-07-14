#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include "../common/protocol.h"
#include <algorithm>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ANSI Color Code for UI
namespace Color {
    const std::string RESET      = "\033[0m";
    const std::string BLACK      = "\033[30m";
    const std::string RED        = "\033[31m";
    const std::string GREEN      = "\033[32m";
    const std::string YELLOW     = "\033[33m";
    const std::string BLUE       = "\033[34m";
    const std::string MAGENTA    = "\033[35m";
    const std::string CYAN       = "\033[36m";
    const std::string WHITE      = "\033[37m";
    
    const std::string BRIGHT_BLACK   = "\033[90m";
    const std::string BRIGHT_RED     = "\033[91m";
    const std::string BRIGHT_GREEN   = "\033[92m";
    const std::string BRIGHT_YELLOW  = "\033[93m";
    const std::string BRIGHT_BLUE    = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string BRIGHT_CYAN    = "\033[96m";
    const std::string BRIGHT_WHITE   = "\033[97m";
    
    const std::string BG_BLACK   = "\033[40m";
    const std::string BG_RED     = "\033[41m";
    const std::string BG_GREEN   = "\033[42m";
    const std::string BG_YELLOW  = "\033[43m";
    const std::string BG_BLUE    = "\033[44m";
    const std::string BG_MAGENTA = "\033[45m";
    const std::string BG_CYAN    = "\033[46m";
    const std::string BG_WHITE   = "\033[47m";
    
    const std::string BOLD       = "\033[1m";
    const std::string DIM        = "\033[2m";
    const std::string ITALIC     = "\033[3m";
    const std::string UNDERLINE  = "\033[4m";
    const std::string BLINK      = "\033[5m";
    const std::string REVERSE    = "\033[7m";
    const std::string HIDDEN     = "\033[8m";
}

// UI helper functions
namespace UI {
    
    std::string boxed(const std::string& text, const std::string& horizontalChar = "═", 
                       const std::string& verticalChar = "║",
                       const std::string& topLeftChar = "╔", const std::string& topRightChar = "╗", 
                       const std::string& bottomLeftChar = "╚", const std::string& bottomRightChar = "╝") {
        size_t width = text.length() + 4;  
        
        std::string result;
        
        result += topLeftChar + std::string(width - 2, *horizontalChar.c_str()) + topRightChar + "\n";
        
        result += verticalChar + " " + text + " " + verticalChar + "\n";
        
        result += bottomLeftChar + std::string(width - 2, *horizontalChar.c_str()) + bottomRightChar + "\n";
        
        return result;
    }
    
    // Draw a header with animation
    void animatedHeader(const std::string& text, int delay = 5, 
                        const std::string& color = Color::BRIGHT_CYAN) {
        int width = 60;
        std::string paddedText = text;
        if (text.length() < static_cast<size_t>(width - 4)) {
            int padLen = (width - 4 - text.length()) / 2;
            paddedText = std::string(padLen, ' ') + text + std::string(padLen, ' ');
            if ((width - 4 - text.length()) % 2 != 0) paddedText += " ";
        }
        
        std::string topBorder    = "╔" + std::string(width - 2, '-') + "╗";
        std::string bottomBorder = "╚" + std::string(width - 2, '-') + "╝";
        std::string middle       = "║" + std::string(width - 2, ' ') + "║";
        
        std::cout << color;
        
        
        for (char& c : topBorder) {
            std::cout << c << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
        std::cout << "\n";
        
        
        std::cout << "║" << std::string((width - 2 - paddedText.length()) / 2, ' ');
        
        
        std::cout << Color::BRIGHT_WHITE << Color::BOLD;
        for (char& c : paddedText) {
            std::cout << c << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
        
        std::cout << color;
        std::cout << std::string((width - 2 - paddedText.length()) / 2, ' ') << "║\n";
        
        
        for (char& c : bottomBorder) {
            std::cout << c << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
        
        std::cout << Color::RESET << "\n";
    }
    
    
    void progressBar(int duration, const std::string& message = "Loading") {
        const int barWidth = 40;
        
        std::cout << message << "\n";
        
        for (int step = 0; step <= barWidth; ++step) {
            float progress = static_cast<float>(step) / barWidth;
            int pos = barWidth * progress;
            
            std::cout << "[";
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << Color::GREEN << "█" << Color::RESET;
                else if (i == pos) std::cout << Color::BRIGHT_GREEN << "▓" << Color::RESET;
                else std::cout << " ";
            }
            
            int percent = static_cast<int>(progress * 100.0);
            std::cout << "] " << percent << "%\r";
            std::cout.flush();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(duration / barWidth));
        }
        std::cout << "\n";
    }
    
    
    void typeText(const std::string& text, int delay = 30, const std::string& color = "") {
        std::cout << color;
        for (char c : text) {
            std::cout << c << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
        if (!color.empty()) std::cout << Color::RESET;
        std::cout << std::endl;
    }
    
    
    void spinner(const std::string& message, int duration) {
        const std::vector<std::string> frames = {
            "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
        };
        
        int numFrames = frames.size();
        int framesPerSecond = 10;
        int totalFrames = (duration * framesPerSecond) / 1000;
        
        for (int i = 0; i < totalFrames; ++i) {
            std::cout << Color::BRIGHT_CYAN << frames[i % numFrames] << " " 
                      << Color::BRIGHT_WHITE << message << Color::RESET << "\r" << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / framesPerSecond));
        }
        std::cout << "  " << message << " Complete!" << std::string(20, ' ') << "\n";
    }
    
    
    std::string rainbowText(const std::string& text) {
        const std::vector<std::string> colors = {
            Color::RED, Color::YELLOW, Color::GREEN, 
            Color::CYAN, Color::BLUE, Color::MAGENTA
        };
        
        std::string result;
        for (size_t i = 0; i < text.length(); ++i) {
            result += colors[i % colors.size()] + std::string(1, text[i]);
        }
        result += Color::RESET;
        
        return result;
    }
}

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
    system("clear");
    std::cout << Color::RESET;
}

void printHeader() {
    std::cout << "\n";
    std::cout << Color::BRIGHT_CYAN << "╔═════════════════════════════════════════════════════════════╗\n";
    std::cout << "║" << Color::BRIGHT_MAGENTA << "  __  __       _ _   _       _                             " << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_MAGENTA << " |  \\/  |_   _| | |_(_)_ __ | | __ _ _   _  ___ _ __      " << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_MAGENTA << " | |\\/| | | | | | __| | '_ \\| |/ _` | | | |/ _ \\ '__| " << Color::BRIGHT_CYAN << "    ║\n";
    std::cout << "║" << Color::BRIGHT_MAGENTA << " | |  | | |_| | | |_| | |_) | | (_| | |_| |  __/ |        " << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_MAGENTA << " |_|  |_|\\__,_|_|\\__|_| .__/|_|\\__,_|\\__, |\\___|_|   " << Color::BRIGHT_CYAN << "     ║\n";
    std::cout << "║" << Color::BRIGHT_MAGENTA << "                       |_|            |___/               " << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_YELLOW << "                  ___        _         ___                 " << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_YELLOW << "                 / _ \\ _   _(_)____   / _ \\__ _ _ __ ___  " << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_YELLOW << "                | | | | | | | |_  /  / /_\\/ _` | '_ ` _ \\ " << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_YELLOW << "                | |_| | |_| | |/ /  / /_\\\\ (_| | | | | | |" << Color::BRIGHT_CYAN << "║\n";
    std::cout << "║" << Color::BRIGHT_YELLOW << "                 \\__\\_\\\\__,_|_/___| \\____/\\__,_|_| |_| |_|" << Color::BRIGHT_CYAN << "║\n";
    std::cout << "╚═════════════════════════════════════════════════════════════╝\n";
    std::cout << Color::RESET << "\n";
}

void printMenu(const std::vector<std::string>& options, const std::string& title = "") {
    const int menuWidth = 50;
    
    
    if (!title.empty()) {
        std::string line = "─";
        std::string titleHeader = " " + title + " ";
        int lineLength = (menuWidth - titleHeader.length()) / 2;
        
        std::cout << Color::BRIGHT_CYAN << "┌" << std::string(menuWidth - 2, '-') << "┐\n";
        std::cout << "│" << Color::RESET << std::string(lineLength, '-') 
                  << Color::BRIGHT_WHITE << Color::BOLD << titleHeader << Color::RESET 
                  << Color::BRIGHT_CYAN << std::string(lineLength, '-');
        
        
        if ((titleHeader.length() + lineLength * 2) < menuWidth - 2) {
            std::cout << "─";
        }
        
        std::cout << Color::BRIGHT_CYAN << "│\n";
    } else {
        std::cout << Color::BRIGHT_CYAN << "┌" << std::string(menuWidth - 2, '-') << "┐\n";
    }
    
    
    std::cout << Color::BRIGHT_CYAN << "│" << Color::RESET << std::string(menuWidth - 2, ' ') << Color::BRIGHT_CYAN << "│\n";
    
    
    for (size_t i = 0; i < options.size(); ++i) {
        std::string optionNumber = std::to_string(i + 1) + ". ";
        std::string optionText = options[i];
        
        std::cout << Color::BRIGHT_CYAN << "│ " << Color::RESET;
        std::cout << Color::BRIGHT_GREEN << optionNumber << Color::RESET;
        
        
        if (i % 3 == 0) {
            std::cout << Color::BRIGHT_WHITE;
        } else if (i % 3 == 1) {
            std::cout << Color::CYAN;
        } else {
            std::cout << Color::BRIGHT_YELLOW;
        }
        
        std::cout << optionText << Color::RESET;
        
        
        int padding = menuWidth - 4 - optionNumber.length() - optionText.length();
        std::cout << std::string(padding, ' ') << Color::BRIGHT_CYAN << "│\n";
    }
    
    
    std::cout << Color::BRIGHT_CYAN << "│" << Color::RESET << std::string(menuWidth - 2, ' ') << Color::BRIGHT_CYAN << "│\n";
    std::cout << "└" << std::string(menuWidth - 2, '-') << "┘\n" << Color::RESET;
    
    
    std::cout << Color::BRIGHT_WHITE << "\n➤ " << Color::RESET << "Enter your choice: ";
}

int getChoice(int maxOptions) {
    int choice;
    std::cout << Color::BRIGHT_CYAN;
    while (!(std::cin >> choice) || choice < 1 || choice > maxOptions) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << Color::BRIGHT_RED << "⚠ Invalid choice. " << Color::RESET 
                  << "Please enter a number between " << Color::BRIGHT_GREEN << "1" << Color::RESET 
                  << " and " << Color::BRIGHT_GREEN << maxOptions << Color::RESET << ": ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << Color::RESET;
    return choice;
}

std::string getInput(const std::string& prompt) {
    std::string input;
    
    std::cout << "\r" << std::string(80, ' ') << "\r"; 
    
    std::cout << Color::BRIGHT_WHITE << "➤ " << Color::RESET << prompt << "\n";
    
    std::cout << Color::BRIGHT_CYAN << "  > " << Color::RESET;
    
    std::getline(std::cin, input);
    std::cout << Color::RESET;
    return input;
}

void printResponse(const ProtocolMessage& parsed) {
    std::cout << "\n" << Color::BRIGHT_BLUE << "┌────────────────────────────────────┐" << Color::RESET << "\n";
    std::cout << Color::BRIGHT_BLUE << "│ " << Color::BRIGHT_WHITE << "Server Response" << Color::BRIGHT_BLUE << "                     │" << Color::RESET << "\n";
    std::cout << Color::BRIGHT_BLUE << "├────────────────────────────────────┤" << Color::RESET << "\n";
    std::cout << Color::BRIGHT_BLUE << "│ " << Color::BRIGHT_YELLOW << "Command: " << Color::BRIGHT_WHITE << parsed.command << Color::RESET;
    
    
    int padding = 34 - 9 - parsed.command.length(); 
    std::cout << std::string(padding, ' ') << Color::BRIGHT_BLUE << "│" << Color::RESET << "\n";
    
    
    if (!parsed.params.empty()) {
        std::cout << Color::BRIGHT_BLUE << "│ " << Color::BRIGHT_YELLOW << "Params: " << Color::RESET;
        int paramPadding = 34 - 8; 
        
        if (parsed.params.size() > 3u) {
            
            std::cout << Color::BRIGHT_BLUE << "                        │" << Color::RESET << "\n";
    for (const auto& param : parsed.params) {
                std::cout << Color::BRIGHT_BLUE << "│   " << Color::BRIGHT_CYAN << "• " << Color::BRIGHT_WHITE << param << Color::RESET;
                int itemPadding = 34 - 5 - param.length(); 
                std::cout << std::string(std::max(0, itemPadding), ' ') << Color::BRIGHT_BLUE << "│" << Color::RESET << "\n";
            }
        } else {
            
            std::string paramStr;
            for (size_t i = 0; i < parsed.params.size(); ++i) {
                paramStr += parsed.params[i];
                if (i < parsed.params.size() - 1) paramStr += ", ";
            }
            
            if (paramStr.length() > static_cast<size_t>(paramPadding - 3)) {
                paramStr = paramStr.substr(0, static_cast<size_t>(paramPadding - 6)) + "...";
            }
            
            std::cout << Color::BRIGHT_WHITE << paramStr << Color::RESET;
            int totalPadding = paramPadding - static_cast<int>(paramStr.length());
            std::cout << std::string(std::max(0, totalPadding), ' ') << Color::BRIGHT_BLUE << "│" << Color::RESET << "\n";
        }
    } else {
        std::cout << Color::BRIGHT_BLUE << "│ " << Color::BRIGHT_YELLOW << "Params: " << Color::DIM << "None" << Color::RESET;
        std::cout << std::string(23, ' ') << Color::BRIGHT_BLUE << "│" << Color::RESET << "\n";
    }
    
    std::cout << Color::BRIGHT_BLUE << "└────────────────────────────────────┘" << Color::RESET << "\n";
}

void printSuccess(const std::string& message) {
    std::cout << "\n" << Color::GREEN << "┌────────────────────────────────────┐" << Color::RESET << "\n";
    std::cout << Color::GREEN << "│ " << Color::BRIGHT_GREEN << "✓ SUCCESS" << Color::GREEN << "                         │" << Color::RESET << "\n";
    std::cout << Color::GREEN << "├────────────────────────────────────┤" << Color::RESET << "\n";
    
    
    std::string remainingMsg = message;
    while (!remainingMsg.empty()) {
        size_t chunkSize = std::min(static_cast<size_t>(32), remainingMsg.length());
        size_t lastSpace = (chunkSize < remainingMsg.length()) ? remainingMsg.substr(0, chunkSize).find_last_of(" ") : std::string::npos;
        
        
        if (lastSpace != std::string::npos && chunkSize < remainingMsg.length()) {
            chunkSize = lastSpace + 1; 
        }
        
        std::string chunk = remainingMsg.substr(0, chunkSize);
        remainingMsg = (chunkSize < remainingMsg.length()) ? remainingMsg.substr(chunkSize) : "";
        
        
        if (!chunk.empty() && chunk[0] == ' ' && chunk != message) {
            chunk = chunk.substr(1);
        }
        
        
        std::cout << Color::GREEN << "│ " << Color::BRIGHT_WHITE << chunk << Color::RESET;
        int padding = 34 - 2 - chunk.length(); 
        std::cout << std::string(std::max(0, padding), ' ') << Color::GREEN << "│" << Color::RESET << "\n";
    }
    
    std::cout << Color::GREEN << "└────────────────────────────────────┘" << Color::RESET << "\n";
}

void printError(const std::string& message) {
    std::cout << "\n" << Color::RED << "┌────────────────────────────────────┐" << Color::RESET << "\n";
    std::cout << Color::RED << "│ " << Color::BRIGHT_RED << "✗ ERROR" << Color::RED << "                           │" << Color::RESET << "\n";
    std::cout << Color::RED << "├────────────────────────────────────┤" << Color::RESET << "\n";
    
    
    std::string remainingMsg = message;
    while (!remainingMsg.empty()) {
        size_t chunkSize = std::min(static_cast<size_t>(32), remainingMsg.length());
        size_t lastSpace = (chunkSize < remainingMsg.length()) ? remainingMsg.substr(0, chunkSize).find_last_of(" ") : std::string::npos;
        
        
        if (lastSpace != std::string::npos && chunkSize < remainingMsg.length()) {
            chunkSize = lastSpace + 1; 
        }
        
        std::string chunk = remainingMsg.substr(0, chunkSize);
        remainingMsg = (chunkSize < remainingMsg.length()) ? remainingMsg.substr(chunkSize) : "";
        
        
        if (!chunk.empty() && chunk[0] == ' ' && chunk != message) {
            chunk = chunk.substr(1);
        }
        
        
        std::cout << Color::RED << "│ " << Color::BRIGHT_WHITE << chunk << Color::RESET;
        int padding = 34 - 2 - chunk.length(); 
        std::cout << std::string(std::max(0, padding), ' ') << Color::RED << "│" << Color::RESET << "\n";
    }
    
    std::cout << Color::RED << "└────────────────────────────────────┘" << Color::RESET << "\n";
}

void waitForEnter() {
    std::cout << "\n" << Color::BRIGHT_CYAN << "Press Enter to continue..." << Color::RESET;
    std::cin.get();
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>


void sendWithNewline(int sock, const std::string& msg) {
    std::string out = msg;
    if (out.empty() || out.back() != '\n') out += '\n';
    send(sock, out.c_str(), static_cast<int>(out.size()), 0);
}


void playGameSession(int sock, const std::string& username, const std::string& currentRoomId, const std::string* initialQuestionRaw) {
    std::string leftover;
    if (initialQuestionRaw && !initialQuestionRaw->empty()) {
        leftover = *initialQuestionRaw;
    }
    bool gameFinished = false;
    
    
    clearScreen();
    printHeader();
    UI::typeText("Preparing game session...", 20, Color::BRIGHT_YELLOW);
    UI::progressBar(1000, "Connecting to game server");
    
    std::cout << Color::BRIGHT_GREEN << "\n┌─────────────────────────────────────────────┐\n";
    std::cout << "│       " << Color::BRIGHT_WHITE << "GAME SESSION ACTIVE - GET READY!" << Color::BRIGHT_GREEN << "       │\n";
    std::cout << "└─────────────────────────────────────────────┘\n" << Color::RESET;
    
    std::cout << "\n" << Color::BRIGHT_CYAN << "Room ID: " << Color::BRIGHT_WHITE << currentRoomId << Color::RESET << "\n";
    std::cout << Color::BRIGHT_MAGENTA << "Player: " << Color::BRIGHT_WHITE << username << Color::RESET << "\n\n";
    
    
    std::cout << Color::BRIGHT_YELLOW;
    for (int i = 3; i > 0; i--) {
        std::cout << "Starting in " << i << "..." << std::string(20, ' ') << "\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
    std::cout << "Game started!" << std::string(20, ' ') << "\n" << Color::RESET;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    int questionCount = 0;
    int score = 0;
    
    while (!gameFinished) {
        // Collect all complete messages in leftover before next recv
        std::vector<std::string> messages;
        while (true) {
            size_t pos = leftover.find('\n');
            if (pos == std::string::npos) break;
            std::string message = leftover.substr(0, pos);
            leftover = leftover.substr(pos + 1);
            if (!message.empty()) messages.push_back(message);
        }
        if (messages.empty()) {
            char buffer[4096];
            int recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (recvLen <= 0) break;
            buffer[recvLen] = '\0';
            leftover += buffer;
            continue;
        }
        // First, process all ANSWER_RESULT messages
        bool answerProcessed = false;
        for (const auto& message : messages) {
            ProtocolMessage parsed = parseMessage(message);
            if (parsed.command == "GAME_RESPONSE" && !parsed.params.empty() && parsed.params[0] == "ANSWER_RESULT") {
                bool correct = (parsed.params.size() > 1u && parsed.params[1] == "CORRECT");
                std::string correctText = (parsed.params.size() > 3u) ? parsed.params[3] : "?";
                std::string scoreStr = (parsed.params.size() > 4u) ? parsed.params[4] : "?";
                score = std::stoi(scoreStr);
                std::cout << "\n";
                if (correct) {
                    std::cout << Color::BRIGHT_GREEN << "┌─────────────────────────────────────────────┐\n";
                    std::cout << "│            " << Color::BRIGHT_WHITE << "✓ CORRECT ANSWER!" << Color::BRIGHT_GREEN << "             │\n";
                    std::cout << "├─────────────────────────────────────────────┤\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "The answer was: " << correctText << Color::RESET;
                    int padding = 41 - 15 - correctText.length();
                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_GREEN << "│\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "Your current score: " << scoreStr << Color::RESET;
                    padding = 41 - 18 - scoreStr.length();
                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_GREEN << "│\n";
                    std::cout << "└─────────────────────────────────────────────┘\n" << Color::RESET;
                    for (int i = 0; i < 3; i++) {
                        std::cout << Color::BRIGHT_GREEN << "+10 points!" << Color::RESET << std::string(20, ' ') << "\r" << std::flush;
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        std::cout << std::string(30, ' ') << "\r" << std::flush;
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                } else {
                    std::cout << Color::BRIGHT_RED << "┌─────────────────────────────────────────────┐\n";
                    std::cout << "│            " << Color::BRIGHT_WHITE << "✗ INCORRECT ANSWER" << Color::BRIGHT_RED << "             │\n";
                    std::cout << "├─────────────────────────────────────────────┤\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "The correct answer was: " << correctText << Color::RESET;
                    int padding = 41 - 24 - correctText.length();
                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_RED << "│\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "Your current score: " << scoreStr << Color::RESET;
                    padding = 41 - 18 - scoreStr.length();
                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_RED << "│\n";
                    std::cout << "└─────────────────────────────────────────────┘\n" << Color::RESET;
                }
                if (parsed.params.size() > 5u && parsed.params[5] == "GAME_FINISHED") {
                    std::cout << "\n" << Color::BRIGHT_MAGENTA;
                    UI::typeText("■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■", 5, Color::BRIGHT_MAGENTA);
                    UI::typeText("             GAME OVER             ", 30, Color::BRIGHT_YELLOW + Color::BOLD);
                    UI::typeText("■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■", 5, Color::BRIGHT_MAGENTA);
                    std::cout << "\n" << Color::BRIGHT_CYAN << "Final Score: " << Color::BRIGHT_WHITE << score << Color::RESET << "\n";
                    if (score > 30) {
                        std::cout << Color::BRIGHT_YELLOW << "\n";
                        std::cout << "       ___________\n";
                        std::cout << "      '._==_==_=_.'\n";
                        std::cout << "      .-\\:      /-.\n";
                        std::cout << "     | (|:.     |) |\n";
                        std::cout << "      '-|:.     |-'\n";
                        std::cout << "        \\::.    /\n";
                        std::cout << "         '::. .'\n";
                        std::cout << "           ) (\n";
                        std::cout << "         _.' '._\n";
                        std::cout << "        `\"\"\"\"\"\"`\n" << Color::RESET;
                        std::cout << Color::BRIGHT_GREEN << "    CONGRATULATIONS!\n" << Color::RESET;
                    }
                    gameFinished = true;
                    break;
                }
                answerProcessed = true;
            }
        }
        if (gameFinished) break;
        // If we just processed an answer, request the next question
        if (answerProcessed && !gameFinished) {
            std::string getQMsg = buildMessage("GET_CURRENT_QUESTION", {username, currentRoomId});
            sendWithNewline(sock, getQMsg);
            // Wait for the next question to arrive in the next loop iteration
            continue;
        }
        // Then process all QUESTION messages
        for (const auto& message : messages) {
            ProtocolMessage parsed = parseMessage(message);
            if (parsed.command == "GAME_RESPONSE" && !parsed.params.empty() && parsed.params[0] == "QUESTION") {
                if (parsed.params.size() < 5u) {
                    std::cout << "\n" << Color::BRIGHT_RED << "[Warning] Malformed question message from server.\n";
                    std::cout << "Raw: ";
                    for (const auto& p : parsed.params) std::cout << "[" << p << "] ";
                    std::cout << "\n" << Color::RESET;
                    gameFinished = true;
                    break;
                }
                questionCount++;
                std::cout << "\n" << Color::BRIGHT_BLUE << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
                std::cout << "┃ " << Color::BRIGHT_YELLOW << "QUESTION " << parsed.params[1] << Color::BRIGHT_CYAN << " | " 
                          << "Time: " << Color::BRIGHT_WHITE << parsed.params[3] << "s" 
                          << Color::BRIGHT_BLUE << std::string(44 - parsed.params[1].length() - parsed.params[3].length(), ' ') << "┃\n";
                std::cout << "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\n";
                std::string question = parsed.params[2];
                int lineWidth = 58;
                for (size_t i = 0; i < question.length(); i += lineWidth) {
                    std::string line = question.substr(i, std::min(lineWidth, static_cast<int>(question.length() - i)));
                    std::cout << "┃ " << Color::BRIGHT_WHITE << line << Color::BRIGHT_BLUE;
                    int padding = lineWidth - line.length();
                    std::cout << std::string(padding, ' ') << "┃\n";
                }
                std::cout << "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\n";
                const std::vector<std::string> optionColors = {
                    Color::BRIGHT_GREEN, Color::BRIGHT_CYAN, 
                    Color::BRIGHT_YELLOW, Color::BRIGHT_MAGENTA
                };
                for (size_t i = 4; i < parsed.params.size(); ++i) {
                    int optionNum = i - 4;
                    std::string optionColor = optionColors[optionNum % optionColors.size()];
                    std::string optionText = parsed.params[i];
                    if (optionText.length() > 2 && optionText[0] >= '1' && optionText[0] <= '9' && 
                        (optionText[1] == '.' || optionText[1] == ')')) {
                        optionText = optionText.substr(2);
                        while (!optionText.empty() && optionText[0] == ' ') {
                            optionText = optionText.substr(1);
                        }
                    }
                    std::cout << "┃ " << optionColor << (optionNum + 1) << ") " << optionText << Color::BRIGHT_BLUE;
                    int padding = lineWidth - optionText.length() - 3; 
                    std::cout << std::string(std::max(0, padding), ' ') << "┃\n";
                }
                std::cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n" << Color::RESET;
                std::string ans;
                std::thread timerThread([&]() {
                    int timeLeft = std::stoi(parsed.params[3]);
                    
                    for (int t = timeLeft-1; t > 0 && ans.empty(); t--) {
                        
                        std::cout << "\033[s";
                        
                        std::cout << "\033[2A";
                        
                        std::cout << "\033[2K";
                        
                        std::cout << Color::BRIGHT_YELLOW << "Time remaining: " << t << "s" << Color::RESET;
                        
                        std::cout << "\033[u" << std::flush;
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                });
                std::cout << "\n\n";
                ans = getInput(Color::BRIGHT_WHITE + "Enter your answer (option number): " + Color::RESET);
                timerThread.detach(); 
                std::cout << std::string(50, ' ') << "\r" << std::flush;
                UI::spinner("Submitting answer", 800);
                    std::string submitMsg = buildMessage("SUBMIT_ANSWER", {username, currentRoomId, ans});
                    sendWithNewline(sock, submitMsg);
                break;
            }
            // Also handle GAME_STARTED as info only
            if (parsed.command == "GAME_RESPONSE" && !parsed.params.empty() && parsed.params[0] == "GAME_STARTED") {
                std::cout << "\n" << Color::BRIGHT_GREEN << "┌─────────────────────────────────────────────┐\n";
                std::cout << "│         " << Color::BRIGHT_WHITE << "GAME HAS OFFICIALLY STARTED" << Color::BRIGHT_GREEN << "         │\n";
                std::cout << "└─────────────────────────────────────────────┘\n" << Color::RESET;
                std::cout << Color::BRIGHT_CYAN << "Players joined: " << Color::RESET;
                for (size_t i = 1; i < parsed.params.size(); ++i) {
                    if (i > 1) std::cout << ", ";
                    if (parsed.params[i] == username) {
                        std::cout << Color::BRIGHT_GREEN << parsed.params[i] << Color::RESET;
                    } else {
                        std::cout << Color::BRIGHT_YELLOW << parsed.params[i] << Color::RESET;
                    }
                    }
                std::cout << "\n\n";
                    continue;
                }
        }
        if (gameFinished) break;
        // ... existing code ...
    }
    waitForEnter();
}


std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    if (start == std::string::npos || end == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}


bool containsWhitespaceOrNewline(const std::string& s) {
    return s.find_first_of(" \t\n\r") != std::string::npos;
}

int main() {
    
    initDebugLog();
    
    
    system("clear");
    
    
    std::cout << Color::BRIGHT_CYAN;
    UI::typeText("Initializing Multiplayer Quiz Game...", 20, Color::BRIGHT_CYAN);
    
    std::cout << Color::BRIGHT_BLUE;
    for (int i = 0; i < 3; i++) {
        std::cout << "■" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "■" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "■" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "■" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "■" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "\r" << std::string(80, ' ') << "\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << Color::RESET;
    
    int sock = -1;
    struct sockaddr_in serverAddr;
    const int PORT = 8080;
    const char* SERVER_IP = "127.0.0.1";
    char buffer[1024];
    int recvLen;
    std::string username;
    std::string currentRoomId;
    bool loggedIn = false;

    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << Color::BRIGHT_RED << "✗ Socket creation failed." << Color::RESET << std::endl;
        return 1;
    }

    
    UI::progressBar(500, "Creating network socket");

    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    UI::typeText("Connecting to quiz server...", 20, Color::BRIGHT_YELLOW);

    
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << Color::BRIGHT_RED << "✗ Connection to server failed." << Color::RESET << std::endl;
        close(sock);
        return 1;
    }

    
    std::cout << Color::BRIGHT_GREEN << "✓ Connected to server successfully!" << Color::RESET << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    
    clearScreen();
    UI::animatedHeader("WELCOME TO MULTIPLAYER QUIZ", 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    
    while (true) {
        clearScreen();
        printHeader();
        
        if (!loggedIn) {
            
            std::cout << "\n";
            std::string authTitle = "AUTHENTICATION REQUIRED";
            
            
            for (int i = 0; i < 3; i++) {
                std::cout << Color::BRIGHT_YELLOW << authTitle << Color::RESET << "\r" << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
                std::cout << Color::YELLOW << authTitle << Color::RESET << "\r" << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            std::cout << Color::BRIGHT_YELLOW << authTitle << Color::RESET << "\n";
            std::cout << Color::BRIGHT_YELLOW << std::string(authTitle.length(), '=') << Color::RESET << "\n\n";
            
            std::vector<std::string> authOptions = {"Register New Account", "Login to Existing Account", "Quit Application"};
            printMenu(authOptions);
            
            int choice = getChoice(authOptions.size());
            
            if (choice == 1) { 
                clearScreen();
                printHeader();
                
                
                std::cout << "\n" << Color::BRIGHT_CYAN << "╔═════════════════════════════════════════╗\n";
                std::cout << "║       " << Color::BRIGHT_WHITE << "CREATE YOUR QUIZ ACCOUNT" << Color::BRIGHT_CYAN << "        ║\n";
                std::cout << "╚═════════════════════════════════════════╝\n\n" << Color::RESET;
                
                while (true) {
                    
                    std::cout << Color::BRIGHT_BLUE << "┌─────────────────────────────────────┐\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "USERNAME" << Color::BRIGHT_BLUE << "                           │\n";
                    std::cout << "└─────────────────────────────────────┘\n" << Color::RESET;
                    username = trim(getInput("  " + Color::BRIGHT_CYAN + "➤ " + Color::RESET + "Enter desired username: "));
                    
                    
                    std::cout << "\n" << Color::BRIGHT_MAGENTA << "┌─────────────────────────────────────┐\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "PASSWORD" << Color::BRIGHT_MAGENTA << "                           │\n";
                    std::cout << "└─────────────────────────────────────┘\n" << Color::RESET;
                    std::string password = trim(getInput("  " + Color::BRIGHT_CYAN + "➤ " + Color::RESET + "Enter secure password: "));
                    
                    if (username.empty() || password.empty()) {
                        printError("Username and password cannot be empty.");
                        continue;
                    }
                    if (containsWhitespaceOrNewline(username) || containsWhitespaceOrNewline(password)) {
                        printError("Username and password must not contain spaces or newlines. Please try again.");
                        continue;
                    }
                    
                    
                    std::cout << "\n";
                    UI::spinner("Creating your account", 1500);
                    
                    debugLogMsg("Registering with username: '" + username + "' password: '" + password + "'");
                    
                    std::string usernameBytes = "Username raw bytes: ";
                    for (size_t i = 0; i < username.size(); ++i) {
                        char hex[4];
                        sprintf(hex, "%02X ", (unsigned char)username[i]);
                        usernameBytes += hex;
                    }
                    debugLogMsg(usernameBytes);
                    
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
                        std::cout << "\n";
                        for (int i = 0; i < 3; i++) {
                            std::cout << Color::BRIGHT_GREEN << "✓ Account created successfully! " << Color::RESET << "\r" << std::flush;
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            std::cout << "                                  \r" << std::flush;
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        }
                        std::cout << Color::BRIGHT_GREEN << "✓ Account created successfully!" << Color::RESET << "\n";
                        std::cout << "\n" << Color::BRIGHT_CYAN << "You can now log in with your credentials." << Color::RESET << "\n";
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
                
            } else if (choice == 2) { 
                clearScreen();
                printHeader();
                
                
                std::cout << "\n" << Color::BRIGHT_GREEN << "╔═════════════════════════════════════════╗\n";
                std::cout << "║         " << Color::BRIGHT_WHITE << "ACCESS YOUR ACCOUNT" << Color::BRIGHT_GREEN << "          ║\n";
                std::cout << "╚═════════════════════════════════════════╝\n\n" << Color::RESET;
                
                
                UI::typeText("Please enter your credentials to continue...", 15, Color::BRIGHT_CYAN);
                std::cout << "\n";
                
                while (true) {
                    
                    std::cout << Color::BRIGHT_BLUE << "┌─────────────────────────────────────┐\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "USERNAME" << Color::BRIGHT_BLUE << "                           │\n";
                    std::cout << "└─────────────────────────────────────┘\n" << Color::RESET;
                    username = trim(getInput("  " + Color::BRIGHT_CYAN + "➤ " + Color::RESET + "Enter your username: "));
                    
                    
                    std::cout << "\n" << Color::BRIGHT_MAGENTA << "┌─────────────────────────────────────┐\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "PASSWORD" << Color::BRIGHT_MAGENTA << "                           │\n";
                    std::cout << "└─────────────────────────────────────┘\n" << Color::RESET;
                    std::string password = trim(getInput("  " + Color::BRIGHT_CYAN + "➤ " + Color::RESET + "Enter your password: "));
                    
                    if (username.empty() || password.empty()) {
                        printError("Username and password cannot be empty.");
                        continue;
                    }
                    if (containsWhitespaceOrNewline(username) || containsWhitespaceOrNewline(password)) {
                        printError("Username and password must not contain spaces or newlines. Please try again.");
                        continue;
                    }
                    
                    
                    std::cout << "\n";
                    UI::spinner("Verifying credentials", 1200);
                    
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
                            
                            
                            std::cout << "\n";
                            for (int i = 0; i < 3; i++) {
                                std::cout << Color::BRIGHT_GREEN << "✓ Login successful! Welcome back, " << Color::BRIGHT_WHITE << username << Color::RESET << "\r" << std::flush;
                                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                                std::cout << "                                                         \r" << std::flush;
                                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            }
                            std::cout << Color::BRIGHT_GREEN << "✓ Login successful! Welcome back, " << Color::BRIGHT_WHITE << username << Color::RESET << "\n";
                            
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
                
            } else if (choice == 3) { 
                std::cout << "\n";
                UI::typeText("Preparing to exit application...", 20, Color::BRIGHT_YELLOW);
                std::string quitMsg = buildMessage("QUIT", {});
                sendWithNewline(sock, quitMsg);
                close(sock);
                
                
                clearScreen();
                std::cout << "\n\n\n";
                std::cout << Color::BRIGHT_CYAN << "╔═════════════════════════════════════════════════╗\n";
                std::cout << "║            " << Color::BRIGHT_WHITE << "THANK YOU FOR PLAYING!" << Color::BRIGHT_CYAN << "             ║\n";
                std::cout << "╚═════════════════════════════════════════════════╝\n\n" << Color::RESET;
                
                UI::typeText("Goodbye! See you next time...", 40, Color::BRIGHT_MAGENTA);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                
                return 0;
            }
            
        } else {
            
            clearScreen();
            printHeader();
            
            
            std::cout << "\n" << Color::BRIGHT_CYAN << "┌─────────────────────────────────────────────┐\n";
            std::cout << "│         " << Color::BRIGHT_WHITE << "WELCOME TO THE QUIZ ARENA" << Color::BRIGHT_CYAN << "         │\n";
            
            if (!currentRoomId.empty()) {
                std::cout << "├─────────────────────────────────────────────┤\n";
                std::cout << "│ " << Color::BRIGHT_YELLOW << "Current Room: " << Color::BRIGHT_WHITE << currentRoomId << Color::RESET;
                
                int padding = 41 - 14 - currentRoomId.length(); 
                std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_CYAN << "│\n";
            }
            
            std::cout << "├─────────────────────────────────────────────┤\n";
            std::cout << "│ " << Color::BRIGHT_GREEN << "Player: " << Color::BRIGHT_WHITE << username << Color::RESET;
            int padding = 41 - 8 - username.length(); 
            std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_CYAN << "│\n";
            std::cout << "└─────────────────────────────────────────────┘\n\n" << Color::RESET;
            
            
            std::cout << Color::BRIGHT_CYAN << "STATUS: " << Color::RESET;
            for (int i = 0; i < 2; i++) {
                std::cout << Color::BRIGHT_GREEN << "ONLINE" << Color::RESET << "\r" << Color::BRIGHT_CYAN << "STATUS: " << Color::RESET << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                std::cout << Color::GREEN << "ONLINE" << Color::RESET << "\r" << Color::BRIGHT_CYAN << "STATUS: " << Color::RESET << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            std::cout << Color::BRIGHT_GREEN << "ONLINE" << Color::RESET << "\n\n";
            
            
            std::vector<std::string> mainOptions = {"Room Management", "Game Actions", "Logout", "Quit"};
            printMenu(mainOptions, "MAIN MENU");
            
            int choice = getChoice(mainOptions.size());
            
            if (choice == 1) { 
                while (true) {
                    clearScreen();
                    printHeader();
                    
                    
                    std::cout << "\n" << Color::BRIGHT_MAGENTA << "╔═════════════════════════════════════════════════╗\n";
                    std::cout << "║              " << Color::BRIGHT_WHITE << "ROOM MANAGEMENT" << Color::BRIGHT_MAGENTA << "               ║\n";
                    std::cout << "╚═════════════════════════════════════════════════╝\n" << Color::RESET;
                    
                    if (!currentRoomId.empty()) {
                        std::cout << "\n" << Color::BRIGHT_YELLOW << "┌─────────────────────────────────┐\n";
                        std::cout << "│ " << Color::BRIGHT_WHITE << "Current Room: " << currentRoomId << Color::RESET;
                        int padding = 29 - 14 - currentRoomId.length();
                        std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_YELLOW << "│\n";
                        std::cout << "└─────────────────────────────────┘\n" << Color::RESET;
                    }
                    
                    std::vector<std::string> roomOptions = {"Create a New Room", "Browse Available Rooms", "Join Existing Room", "Back to Main Menu"};
                    printMenu(roomOptions);
                    
                    int roomChoice = getChoice(roomOptions.size());
                    
                    if (roomChoice == 1) { 
                        std::cout << "\n" << Color::BRIGHT_BLUE << "┌─────────────────────────────────┐\n";
                        std::cout << "│ " << Color::BRIGHT_WHITE << "CREATE NEW QUIZ ROOM" << Color::BRIGHT_BLUE << "           │\n";
                        std::cout << "└─────────────────────────────────┘\n" << Color::RESET;
                        
                        
                        std::string roomName = getInput(Color::BRIGHT_CYAN + "Enter a name for your new room: " + Color::RESET);
                        
                        
                        std::cout << "\n";
                        UI::spinner("Creating your room", 1200);
                        
                        std::string createRoomMsg = buildMessage("CREATE_ROOM", {username, roomName});
                        sendWithNewline(sock, createRoomMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                            if (parsed.command == "OK" && parsed.params.size() >= 2u) {
                                currentRoomId = parsed.params[1];
                                
                                
                                std::cout << Color::BRIGHT_GREEN << "\n┌─────────────────────────────────────────────┐\n";
                                std::cout << "│ " << Color::BRIGHT_WHITE << "Room \"" << roomName << "\" created successfully!" << Color::RESET;
                                int padding = 41 - 9 - roomName.length() - 21;
                                std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_GREEN << "│\n";
                                std::cout << "│ " << Color::BRIGHT_WHITE << "Room ID: " << currentRoomId << Color::RESET;
                                padding = 41 - 9 - currentRoomId.length();
                                std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_GREEN << "│\n";
                                std::cout << "└─────────────────────────────────────────────┘\n" << Color::RESET;
                                
                                
                                for (int i = 0; i < 3; i++) {
                                    std::cout << Color::BRIGHT_CYAN << "You are now the host of this room!" << Color::RESET << "\r" << std::flush;
                                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                                    std::cout << "                                  \r" << std::flush;
                                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                                }
                                std::cout << Color::BRIGHT_CYAN << "You are now the host of this room!" << Color::RESET << "\n\n";
                                
                                
                                std::cout << Color::BRIGHT_YELLOW << "Redirecting to game actions..." << Color::RESET << "\n";
                                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                                
                                
                                goto gameActionsMenu;
                            } else {
                                printError("Failed to create room.");
                                waitForEnter();
                            }
                        } else {
                        waitForEnter();
                        }
                        
                    } else if (roomChoice == 2) { 
                        
                        std::cout << "\n";
                        UI::spinner("Fetching available rooms", 1000);
                        
                        std::string browseMsg = buildMessage("BROWSE_ROOMS", {});
                        sendWithNewline(sock, browseMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            
                            std::cout << "\n" << Color::BRIGHT_CYAN << "╔═════════════════════════════════════════════════╗\n";
                            std::cout << "║              " << Color::BRIGHT_WHITE << "AVAILABLE ROOMS" << Color::BRIGHT_CYAN << "                ║\n";
                            std::cout << "╠═════════════════════════════════════════════════╣\n";
                            
                            if (parsed.params.empty()) {
                                std::cout << "║ " << Color::BRIGHT_YELLOW << "No rooms available. Create one to get started!" << Color::BRIGHT_CYAN << " ║\n";
                            } else {
                            for (size_t i = 0; i + 1 < parsed.params.size(); i += 2) {
                                    std::string roomId = parsed.params[i];
                                    std::string roomName = parsed.params[i+1];
                                    
                                    std::cout << "║ " << Color::BRIGHT_GREEN << "ID: " << Color::BRIGHT_WHITE << roomId 
                                              << Color::BRIGHT_YELLOW << " | " << Color::BRIGHT_GREEN << "Name: " 
                                              << Color::BRIGHT_WHITE << roomName << Color::RESET;
                                    
                                    
                                    int padding = 49 - 10 - roomId.length() - roomName.length();
                                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_CYAN << "║\n";
                                }
                            }
                            
                            std::cout << "╚═════════════════════════════════════════════════╝\n" << Color::RESET;
                        }
                        waitForEnter();
                        
                    } else if (roomChoice == 3) { 
                        std::cout << "\n" << Color::BRIGHT_GREEN << "┌─────────────────────────────────┐\n";
                        std::cout << "│ " << Color::BRIGHT_WHITE << "JOIN EXISTING ROOM" << Color::BRIGHT_GREEN << "             │\n";
                        std::cout << "└─────────────────────────────────┘\n" << Color::RESET;
                        
                        std::string roomIdStr = getInput(Color::BRIGHT_CYAN + "Enter room ID to join: " + Color::RESET);
                        
                        
                        std::cout << "\n";
                        UI::spinner("Joining room", 1200);
                        
                        std::string joinMsg = buildMessage("JOIN_ROOM", {username, roomIdStr});
                        sendWithNewline(sock, joinMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                            if (parsed.command == "OK") {
                                currentRoomId = roomIdStr;
                                
                                
                                std::cout << Color::BRIGHT_GREEN << "\n┌─────────────────────────────────────────────┐\n";
                                std::cout << "│ " << Color::BRIGHT_WHITE << "Successfully joined room: " << roomIdStr << Color::RESET;
                                int padding = 41 - 25 - roomIdStr.length();
                                std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_GREEN << "│\n";
                                std::cout << "└─────────────────────────────────────────────┘\n" << Color::RESET;
                                
                                
                                std::cout << "\n" << Color::BRIGHT_YELLOW << "Waiting for the game to start..." << Color::RESET << "\n\n";
                                
                                
                                const std::vector<std::string> spinChars = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
                                int spinIdx = 0;
                                bool receivedMessage = false;
                                
                                
                                int flags = fcntl(sock, F_GETFL, 0);
                                fcntl(sock, F_SETFL, flags | O_NONBLOCK);
                                
                                
                                time_t startTime = time(NULL);
                                while (!receivedMessage && (time(NULL) - startTime) < 30) { 
                                    std::cout << Color::BRIGHT_CYAN << spinChars[spinIdx] << " " 
                                              << Color::BRIGHT_WHITE << "Waiting for game to start..." 
                                              << Color::RESET << "\r" << std::flush;
                                    spinIdx = (spinIdx + 1) % spinChars.size();
                                    
                                    
                                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                    
                                    recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                                    if (recvLen > 0) {
                                        buffer[recvLen] = '\0';
                                        ProtocolMessage waitMsg = parseMessage(buffer);
                                        receivedMessage = true;
                                        
                                        
                                        std::cout << std::string(60, ' ') << "\r" << std::flush;
                                        
                                        if (waitMsg.command == "QUESTION") {
                                            
                                            std::string questionRaw = buffer;
                                            playGameSession(sock, username, currentRoomId, &questionRaw);
                                            break;
                                        } else if (waitMsg.command == "GAME_RESPONSE" && !waitMsg.params.empty() && waitMsg.params[0] == "QUESTION") {
                                            
                                            std::string questionRaw = buffer;
                                            playGameSession(sock, username, currentRoomId, &questionRaw);
                                            break;
                                        } else if (waitMsg.command == "ERROR") {
                                            printError(waitMsg.params.empty() ? "Game error" : waitMsg.params[0]);
                                            break;
                                        } else {
                                            
                                            printResponse(waitMsg);
                                        }
                                    }
                                }
                                
                                
                                fcntl(sock, F_SETFL, flags);
                                
                                if (!receivedMessage) {
                                    std::cout << Color::BRIGHT_YELLOW << "\nStill waiting for the game to start... Press Enter to return to menu." << Color::RESET << "\n";
                                }
                            } else {
                                printError("Failed to join room.");
                            }
                        }
                        waitForEnter();
                        
                    } else if (roomChoice == 4) { 
                        break;
                    }
                }
                
            } else if (choice == 2) { 
                if (currentRoomId.empty()) {
                    printError("You must join or create a room first.");
                    waitForEnter();
                    continue;
                }
                
                
                gameActionsMenu:
                
                while (true) {
                    clearScreen();
                    printHeader();
                    
                    
                    std::cout << "\n" << Color::BRIGHT_YELLOW << "╔═════════════════════════════════════════════════╗\n";
                    std::cout << "║               " << Color::BRIGHT_WHITE << "GAME ACTIONS" << Color::BRIGHT_YELLOW << "                 ║\n";
                    std::cout << "╚═════════════════════════════════════════════════╝\n" << Color::RESET;
                    
                    
                    std::cout << "\n" << Color::BRIGHT_CYAN << "┌─────────────────────────────────┐\n";
                    std::cout << "│ " << Color::BRIGHT_WHITE << "Active Room: " << currentRoomId << Color::RESET;
                    int padding = 29 - 13 - currentRoomId.length();
                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_CYAN << "│\n";
                    std::cout << "└─────────────────────────────────┘\n\n" << Color::RESET;
                    
                    
                    std::vector<std::string> gameOptions = {"Start Game", "Game Information", "View Leaderboard", "End Current Game", "Back to Main Menu"};
                    printMenu(gameOptions);
                    
                    int gameChoice = getChoice(gameOptions.size());
                    
                    if (gameChoice == 1) { 
                        std::cout << "\n" << Color::BRIGHT_GREEN << "┌─────────────────────────────────┐\n";
                        std::cout << "│ " << Color::BRIGHT_WHITE << "GAME SETUP" << Color::BRIGHT_GREEN << "                    │\n";
                        std::cout << "└─────────────────────────────────┘\n" << Color::RESET;
                        
                        
                        std::cout << "\n" << Color::BRIGHT_WHITE << "Select number of questions:" << Color::RESET << "\n";
                        
                        int defaultQuestions = 5;
                        int minQuestions = 1;
                        int maxQuestions = 10;
                        
                        
                        std::cout << Color::BRIGHT_BLUE << "  " << minQuestions;
                        std::cout << std::string(5, ' ') << Color::RESET;
                        
                        for (int i = minQuestions; i <= maxQuestions; i++) {
                            if (i == defaultQuestions) {
                                std::cout << Color::BRIGHT_GREEN << "◉" << Color::RESET;
                            } else {
                                std::cout << Color::DIM << "○" << Color::RESET;
                            }
                            std::cout << " ";
                        }
                        
                        std::cout << Color::BRIGHT_BLUE << std::string(5, ' ') << maxQuestions << Color::RESET << "\n\n";
                        
                        std::string qCount = getInput(Color::BRIGHT_CYAN + "How many questions? [default: 5]: " + Color::RESET);
                        if (qCount.empty()) qCount = "5";
                        
                        
                        std::cout << "\n" << Color::BRIGHT_YELLOW << "Starting game with " << Color::BRIGHT_WHITE << qCount << Color::BRIGHT_YELLOW << " questions..." << Color::RESET << "\n";
                        
                        
                        UI::spinner("Initializing game", 1500);
                        
                        std::string startMsg = buildMessage("START_GAME", {username, currentRoomId, qCount});
                        sendWithNewline(sock, startMsg);
                        char buffer[4096];
                        int recvLen = 0;
                        std::string leftover;
                        bool gotQuestion = false;
                        // Read all messages until we get a QUESTION
                        while (!gotQuestion) {
                            recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                            if (recvLen <= 0) break;
                            buffer[recvLen] = '\0';
                            leftover += buffer;
                            // Check if QUESTION is in the buffer
                            if (leftover.find("QUESTION") != std::string::npos) {
                                gotQuestion = true;
                            }
                            // Optionally, break if buffer contains at least one newline (one complete message)
                            if (leftover.find('\n') != std::string::npos) {
                                break;
                            }
                        }
                        playGameSession(sock, username, currentRoomId, leftover.empty() ? nullptr : &leftover);
                        waitForEnter();
                        
                    } else if (gameChoice == 2) { 
                        
                        std::cout << "\n";
                        UI::spinner("Retrieving game information", 800);
                        
                        std::string infoMsg = buildMessage("GET_GAME_INFO", {username, currentRoomId});
                        sendWithNewline(sock, infoMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            
                            
                            std::cout << "\n" << Color::BRIGHT_CYAN << "╔═════════════════════════════════════════════════╗\n";
                            std::cout << "║              " << Color::BRIGHT_WHITE << "GAME INFORMATION" << Color::BRIGHT_CYAN << "               ║\n";
                            std::cout << "╠═════════════════════════════════════════════════╣\n";
                            
                            if (parsed.params.empty()) {
                                std::cout << "║ " << Color::BRIGHT_YELLOW << "No game information available." << Color::BRIGHT_CYAN << "             ║\n";
                            } else {
                                for (size_t i = 0; i < parsed.params.size(); ++i) {
                                    
                                    size_t colonPos = parsed.params[i].find(':');
                                    if (colonPos != std::string::npos) {
                                        std::string key = parsed.params[i].substr(0, colonPos);
                                        std::string value = parsed.params[i].substr(colonPos + 1);
                                        
                                        std::cout << "║ " << Color::BRIGHT_GREEN << key << ": " << Color::BRIGHT_WHITE << value << Color::RESET;
                                    } else {
                                        std::cout << "║ " << Color::BRIGHT_WHITE << parsed.params[i] << Color::RESET;
                                    }
                                    
                                    
                                    int padding = 49 - parsed.params[i].length();
                                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_CYAN << "║\n";
                                }
                            }
                            
                            std::cout << "╚═════════════════════════════════════════════════╝\n" << Color::RESET;
                        }
                        waitForEnter();
                        
                    } else if (gameChoice == 3) { // View Leaderboard
                        std::cout << "\n";
                        UI::spinner("Fetching leaderboard", 800);
                        std::string leaderboardMsg = buildMessage("GET_LEADERBOARD", {username, currentRoomId});
                        sendWithNewline(sock, leaderboardMsg);
                        char buffer[4096];
                        int recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            if (parsed.command == "LEADERBOARD") {
                                // Display leaderboard as in playGameSession
                                std::cout << "\n" << Color::BRIGHT_MAGENTA << "╔════════════════════════════════════════════╗\n";
                                std::cout << "║            " << Color::BRIGHT_WHITE << "FINAL LEADERBOARD" << Color::BRIGHT_MAGENTA << "             ║\n";
                                std::cout << "╠════════════════════════════════════════════╣\n";
                                std::vector<std::pair<std::string, int>> leaderboardEntries;
                                std::string leaderboardStr = parsed.params[0];
                                if (leaderboardStr.empty() || leaderboardStr == "LEADERBOARD:") {
                                    leaderboardEntries.push_back({username, 0});
                                } else {
                                    std::istringstream iss(leaderboardStr);
                                    std::string line;
                                    while (std::getline(iss, line)) {
                                        size_t pos = line.find(':');
                                        if (pos != std::string::npos) {
                                            std::string name = line.substr(0, pos);
                                            std::string scoreText = line.substr(pos + 1);
                                            scoreText.erase(std::remove_if(scoreText.begin(), scoreText.end(), [](unsigned char c) { return std::isspace(c); }), scoreText.end());
                                            int score = std::stoi(scoreText);
                                            leaderboardEntries.push_back({name, score});
                                        }
                                    }
                                }
                                std::sort(leaderboardEntries.begin(), leaderboardEntries.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) { return a.second > b.second; });
                                const std::vector<std::string> medals = {"🥇", "🥈", "🥉"};
                                for (size_t i = 0; i < leaderboardEntries.size(); ++i) {
                                    std::string position = (i < 3) ? medals[i] + " " : std::to_string(i + 1) + ". ";
                                    std::string name = leaderboardEntries[i].first;
                                    std::string scoreStr = std::to_string(leaderboardEntries[i].second);
                                    std::string nameColor = (name == username) ? Color::BRIGHT_GREEN : Color::BRIGHT_WHITE;
                                    std::cout << "║ " << Color::BRIGHT_YELLOW << position << nameColor << name << Color::RESET;
                                    int padding = 40 - position.length() - name.length() - scoreStr.length();
                                    std::cout << std::string(std::max(0, padding), ' ') << Color::BRIGHT_CYAN << scoreStr << Color::BRIGHT_MAGENTA << " ║\n";
                                }
                                std::cout << "╚════════════════════════════════════════════╝\n" << Color::RESET;
                            } else if (parsed.command == "ERROR") {
                                printError(parsed.params.empty() ? "Could not fetch leaderboard." : parsed.params[0]);
                            } else {
                            printResponse(parsed);
                            }
                        } else {
                            printError("No response from server.");
                        }
                        waitForEnter();
                    } else if (gameChoice == 4) { 
                        
                        std::cout << "\n" << Color::BRIGHT_RED << "┌─────────────────────────────────┐\n";
                        std::cout << "│ " << Color::BRIGHT_WHITE << "WARNING: END CURRENT GAME" << Color::BRIGHT_RED << "       │\n";
                        std::cout << "└─────────────────────────────────┘\n" << Color::RESET;
                        
                        std::cout << "\n" << Color::BRIGHT_YELLOW << "Are you sure you want to end the current game?\n";
                        std::cout << "This action cannot be undone." << Color::RESET << "\n\n";
                        
                        std::string confirmation = getInput(Color::BRIGHT_RED + "Type 'yes' to confirm: " + Color::RESET);
                        
                        if (confirmation == "yes" || confirmation == "YES" || confirmation == "Yes") {
                            
                            std::cout << "\n";
                            UI::spinner("Ending game", 1000);
                            
                        std::string endMsg = buildMessage("END_GAME", {username, currentRoomId});
                        sendWithNewline(sock, endMsg);
                        
                        recvLen = recv(sock, buffer, sizeof(buffer) - 1, 0);
                        if (recvLen > 0) {
                            buffer[recvLen] = '\0';
                            ProtocolMessage parsed = parseMessage(buffer);
                            printResponse(parsed);
                                
                                if (parsed.command == "OK") {
                                    
                                    std::cout << "\n" << Color::BRIGHT_GREEN << "Game ended successfully!" << Color::RESET << "\n";
                                }
                            }
                        } else {
                            std::cout << "\n" << Color::BRIGHT_CYAN << "Game end cancelled." << Color::RESET << "\n";
                        }
                        // waitForEnter();
                        
                    } else if (gameChoice == 5) { 
                        break;
                    }
                }
                
            } else if (choice == 3) { 
                
                std::cout << "\n";
                UI::spinner("Logging out", 1000);
                
                loggedIn = false;
                username.clear();
                currentRoomId.clear();
                
                
                std::cout << "\n" << Color::BRIGHT_GREEN << "┌─────────────────────────────────────────────┐\n";
                std::cout << "│         " << Color::BRIGHT_WHITE << "LOGGED OUT SUCCESSFULLY" << Color::BRIGHT_GREEN << "           │\n";
                std::cout << "└─────────────────────────────────────────────┘\n" << Color::RESET;
                
                waitForEnter();
                
            } else if (choice == 4) { 
                
                std::cout << "\n";
                UI::typeText("Preparing to exit application...", 20, Color::BRIGHT_YELLOW);
                std::string quitMsg = buildMessage("QUIT", {});
                sendWithNewline(sock, quitMsg);
                
                
                UI::progressBar(800, "Closing network connection");
                close(sock);
                
                
                clearScreen();
                std::cout << "\n\n\n";
                std::cout << Color::BRIGHT_CYAN << "╔═════════════════════════════════════════════════╗\n";
                std::cout << "║            " << Color::BRIGHT_WHITE << "THANK YOU FOR PLAYING!" << Color::BRIGHT_CYAN << "             ║\n";
                std::cout << "╚═════════════════════════════════════════════════╝\n\n" << Color::RESET;
                
                UI::typeText("Goodbye! See you next time...", 40, Color::BRIGHT_MAGENTA);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                
                return 0;
            }
        }
    }
    close(sock);
    closeDebugLog();
    return 0;
} 