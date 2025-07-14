#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <string>
#include <map>
#include <vector>
#include "../common/user.h"
#include "../common/game_state.h"

class AuthenticationManager {
private:
    std::map<std::string, User> users;  
    std::string userDataFile;  

public:
    AuthenticationManager(const std::string& dataFile = "data/users.txt");
    ~AuthenticationManager();

    bool registerUser(const std::string& username, const std::string& password);
    bool authenticateUser(const std::string& username, const std::string& password);
    bool userExists(const std::string& username) const;
    
    User* getUser(const std::string& username);
    bool updateUser(const User& user);
    std::vector<std::string> getAllUsernames() const;
    
    // Data persistence
    bool loadUsersFromFile();
    bool saveUsersToFile() const;
    
    bool createAdminUser(const std::string& username, const std::string& password);
    bool isAdmin(const std::string& username) const;
    
    int getUserCount() const { return static_cast<int>(users.size()); }
    void clearUsers() { users.clear(); }
};

#endif