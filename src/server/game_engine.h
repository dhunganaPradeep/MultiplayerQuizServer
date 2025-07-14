#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include "room_manager.h"
#include "question_manager.h"

struct PlayerScore {
    std::string username;
    int score;
    int correctAnswers;
    int totalAnswers;
    std::chrono::steady_clock::time_point lastAnswerTime;
    
    PlayerScore() : score(0), correctAnswers(0), totalAnswers(0) {}
};

struct GameSession {
    enum State {
        WAITING,
        PLAYING,
        FINISHED
    };
    
    State currentState;
    int currentQuestionIndex; 
    int totalQuestions;
    std::chrono::steady_clock::time_point roundStartTime;
    std::chrono::steady_clock::time_point questionStartTime;
    int roundTimeLimit; 
    int questionTimeLimit;
    int gameDurationSeconds;
    std::chrono::steady_clock::time_point gameStartTime;
    std::map<std::string, int> playerQuestionIndex;
    
    GameSession() : currentState(WAITING), currentQuestionIndex(0), 
                    totalQuestions(0), roundTimeLimit(300), questionTimeLimit(30),
                    gameDurationSeconds(90) {}
};

class GameEngine {
private:
    std::map<int, GameSession> gameSessions; 
    std::map<int, std::map<std::string, PlayerScore>> playerScores; // roomId -> {username -> score}
    std::map<int, std::vector<Question>> roomQuestions; 
    std::map<int, std::vector<std::string>> roomPlayers; 
    
    RoomManager& roomManager;
    QuestionManager& questionManager;
    
    bool isGameActive(int roomId);
    void startNewRound(int roomId);
    void endRound(int roomId);
    std::string getGameStatus(int roomId);
    std::string getLeaderboard(int roomId);
    void awardPoints(int roomId, const std::string& username, bool correct, int timeBonus = 0);
    
public:
    GameEngine(RoomManager& rm, QuestionManager& qm);
    ~GameEngine();
    
    // Game control
    std::string startGame(int roomId, const std::string& username, int questionCount = 10);
    std::string endGame(int roomId, const std::string& username);
    std::string getCurrentQuestion(int roomId, const std::string& username);
    std::string submitAnswer(int roomId, const std::string& username, int answerIndex);
    std::string getGameInfo(int roomId, const std::string& username);
    std::string getLeaderboard(int roomId, const std::string& username);
    
    // Game state queries
    bool isPlayerInGame(int roomId, const std::string& username);
    bool canStartGame(int roomId, const std::string& username);
    int getPlayerCount(int roomId);
    std::vector<std::string> getActivePlayers(int roomId);
    
    void removePlayer(int roomId, const std::string& username);
    void cleanupRoom(int roomId);

    bool isGameTimerExpired(int roomId);
};

#endif