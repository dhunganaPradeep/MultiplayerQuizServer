#ifndef USER_H
#define USER_H

#include <string>

struct User {
    std::string username;
    std::string password;
    int currentRoomId;  
    int score;
    bool isAdmin;

    User() : currentRoomId(-1), score(0), isAdmin(false) {}
    User(const std::string& uname, const std::string& pwd) 
        : username(uname), password(pwd), currentRoomId(-1), score(0), isAdmin(false) {}

    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; }
    int getCurrentRoomId() const { return currentRoomId; }
    int getScore() const { return score; }
    bool getIsAdmin() const { return isAdmin; }

    void setCurrentRoomId(int roomId) { currentRoomId = roomId; }
    void setScore(int newScore) { score = newScore; }
    void setIsAdmin(bool admin) { isAdmin = admin; }
    void addScore(int points) { score += points; }
};

#endif