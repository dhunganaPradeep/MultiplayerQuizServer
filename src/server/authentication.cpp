#include "authentication.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "debug_log.h"
#include <ctime>

AuthenticationManager::AuthenticationManager(const std::string& dataFile) 
    : userDataFile(dataFile) {
    loadUsersFromFile();
    if (users.empty()) {
        createAdminUser("admin", "admin123");
    }
}

AuthenticationManager::~AuthenticationManager() {
    saveUsersToFile();
}

bool containsWhitespaceOrNewline(const std::string& s) {
    return s.find_first_of(" \t\n\r") != std::string::npos;
}

bool AuthenticationManager::registerUser(const std::string& username, const std::string& password) {
    debugLogMsg("Attempting to register username: '" + username + "'");
    std::string currentUsers = "Current users: ";
    for (const auto& pair : users) currentUsers += "'" + pair.first + "' ";
    debugLogMsg(currentUsers);
    if (username.empty() || password.empty()) {
        return false;
    }
    if (containsWhitespaceOrNewline(username) || containsWhitespaceOrNewline(password)) {
        debugLogMsg("ERROR: Username and password must not contain spaces or newlines.");
        return false;
    }
    
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    
    if (password.length() < 3) {
        return false;
    }
    
    debugLogMsg("Checking if username exists: '" + username + "'");
    if (userExists(username)) {
        debugLogMsg("Username already exists: '" + username + "'");
        return false;
    }
    
    User newUser(username, password);
    users[username] = newUser;
    
    saveUsersToFile();
    
    std::cout << "User registered: " << username << std::endl;
    return true;
}

bool AuthenticationManager::authenticateUser(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    return it->second.getPassword() == password;
}

bool AuthenticationManager::userExists(const std::string& username) const {
    debugLogMsg("userExists called for: '" + username + "'");
    for (const auto& pair : users) {
        debugLogMsg("Comparing to: '" + pair.first + "'");
    }
    return users.find(username) != users.end();
}

User* AuthenticationManager::getUser(const std::string& username) {
    auto it = users.find(username);
    if (it != users.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool AuthenticationManager::updateUser(const User& user) {
    auto it = users.find(user.getUsername());
    if (it != users.end()) {
        it->second = user;
        saveUsersToFile();
        return true;
    }
    return false;
}

std::vector<std::string> AuthenticationManager::getAllUsernames() const {
    std::vector<std::string> usernames;
    for (const auto& pair : users) {
        usernames.push_back(pair.first);
    }
    return usernames;
}

bool AuthenticationManager::loadUsersFromFile() {
    std::ifstream file(userDataFile);
    if (!file.is_open()) {
        std::cout << "No existing user file found. Starting fresh." << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password;
        int currentRoomId, score;
        bool isAdmin;
        
        if (iss >> username >> password >> currentRoomId >> score >> isAdmin) {
            User user(username, password);
            user.setCurrentRoomId(currentRoomId);
            user.setScore(score);
            user.setIsAdmin(isAdmin);
            users[username] = user;
        }
    }
    
    file.close();
    std::cout << "Loaded " << users.size() << " users from file." << std::endl;
    return true;
}

bool AuthenticationManager::saveUsersToFile() const {
    std::ofstream file(userDataFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open user file for writing." << std::endl;
        return false;
    }
    
    for (const auto& pair : users) {
        const User& user = pair.second;
        file << user.getUsername() << " " 
             << user.getPassword() << " "
             << user.getCurrentRoomId() << " "
             << user.getScore() << " "
             << (user.getIsAdmin() ? 1 : 0) << std::endl;
    }
    
    file.close();
    return true;
}

bool AuthenticationManager::createAdminUser(const std::string& username, const std::string& password) {
    if (registerUser(username, password)) {
        User* user = getUser(username);
        if (user) {
            user->setIsAdmin(true);
            saveUsersToFile();
            std::cout << "Admin user created: " << username << std::endl;
            return true;
        }
    }
    return false;
}

bool AuthenticationManager::isAdmin(const std::string& username) const {
    auto it = users.find(username);
    if (it != users.end()) {
        return it->second.getIsAdmin();
    }
    return false;
} 