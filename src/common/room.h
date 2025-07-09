#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "user.h"

enum class GameState {
    WAITING,    // Waiting for players to join
    PLAYING,    // Quiz is in progress
    FINISHED    // Quiz is finished
};

struct Room {
    int roomId;
    std::string roomName;
    std::string hostUsername;
    std::vector<std::string> players;  // List of usernames
    std::map<std::string, int> playerScores;  // username -> score
    int currentQuestionIndex;
    int totalQuestions;
    GameState gameState;

    Room() : roomId(-1), currentQuestionIndex(-1), totalQuestions(0), gameState(GameState::WAITING) {}
    Room(int id, const std::string& name, const std::string& host)
        : roomId(id), roomName(name), hostUsername(host), currentQuestionIndex(-1), 
          totalQuestions(0), gameState(GameState::WAITING) {}

    // Basic getters
    int getRoomId() const { return roomId; }
    std::string getRoomName() const { return roomName; }
    std::string getHostUsername() const { return hostUsername; }
    std::vector<std::string> getPlayers() const { return players; }
    int getCurrentQuestionIndex() const { return currentQuestionIndex; }
    int getTotalQuestions() const { return totalQuestions; }
    GameState getGameState() const { return gameState; }

    // Player management
    void addPlayer(const std::string& username) {
        if (std::find(players.begin(), players.end(), username) == players.end()) {
            players.push_back(username);
            playerScores[username] = 0;
        }
    }

    void removePlayer(const std::string& username) {
        players.erase(std::remove(players.begin(), players.end(), username), players.end());
        playerScores.erase(username);
    }

    bool hasPlayer(const std::string& username) const {
        return std::find(players.begin(), players.end(), username) != players.end();
    }

    int getPlayerCount() const { return static_cast<int>(players.size()); }

    // Score management
    void setPlayerScore(const std::string& username, int score) {
        playerScores[username] = score;
    }

    void addPlayerScore(const std::string& username, int points) {
        playerScores[username] += points;
    }

    int getPlayerScore(const std::string& username) const {
        auto it = playerScores.find(username);
        return (it != playerScores.end()) ? it->second : 0;
    }

    // Game state management
    void setGameState(GameState state) { gameState = state; }
    void setCurrentQuestionIndex(int index) { currentQuestionIndex = index; }
    void setTotalQuestions(int total) { totalQuestions = total; }

    bool isGameInProgress() const { return gameState == GameState::PLAYING; }
    bool isGameFinished() const { return gameState == GameState::FINISHED; }
    bool isWaiting() const { return gameState == GameState::WAITING; }
};

#endif // ROOM_H 