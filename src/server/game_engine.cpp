#include "game_engine.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

GameEngine::GameEngine(RoomManager& rm, QuestionManager& qm) 
    : roomManager(rm), questionManager(qm) {
}

GameEngine::~GameEngine() {
}

bool GameEngine::isGameActive(int roomId) {
    auto it = gameSessions.find(roomId);
    return it != gameSessions.end() && it->second.currentState == GameSession::PLAYING;
}

void GameEngine::startNewRound(int roomId) {
    auto& gameSession = gameSessions[roomId];
    gameSession.currentState = GameSession::PLAYING;
    gameSession.currentQuestionIndex = 0;
    gameSession.roundStartTime = std::chrono::steady_clock::now();
    gameSession.questionStartTime = std::chrono::steady_clock::now();
}

void GameEngine::endRound(int roomId) {
    auto& gameSession = gameSessions[roomId];
    gameSession.currentState = GameSession::FINISHED;
}

std::string GameEngine::getGameStatus(int roomId) {
    auto it = gameSessions.find(roomId);
    if (it == gameSessions.end()) {
        return "NO_GAME";
    }
    
    const auto& gameSession = it->second;
    std::ostringstream oss;
    
    switch (gameSession.currentState) {
        case GameSession::WAITING:
            oss << "WAITING";
            break;
        case GameSession::PLAYING:
            oss << "PLAYING|" << gameSession.currentQuestionIndex + 1 << "/" << gameSession.totalQuestions;
            break;
        case GameSession::FINISHED:
            oss << "FINISHED";
            break;
    }
    
    return oss.str();
}

std::string GameEngine::getLeaderboard(int roomId) {
    auto it = playerScores.find(roomId);
    if (it == playerScores.end()) {
        return "NO_SCORES";
    }
    
    std::vector<std::pair<std::string, PlayerScore>> sortedScores;
    for (const auto& pair : it->second) {
        sortedScores.push_back(pair);
    }
    
    // Sort by score (descending), then by correct answers, then by username
    std::sort(sortedScores.begin(), sortedScores.end(), 
        [](const auto& a, const auto& b) {
            if (a.second.score != b.second.score) {
                return a.second.score > b.second.score;
            }
            if (a.second.correctAnswers != b.second.correctAnswers) {
                return a.second.correctAnswers > b.second.correctAnswers;
            }
            return a.first < b.first;
        });
    
    std::ostringstream oss;
    oss << "LEADERBOARD";
    for (size_t i = 0; i < sortedScores.size(); ++i) {
        const auto& player = sortedScores[i];
        oss << "|" << (i + 1) << "." << player.first << ":" 
            << player.second.score << "(" << player.second.correctAnswers 
            << "/" << player.second.totalAnswers << ")";
    }
    
    return oss.str();
}

void GameEngine::awardPoints(int roomId, const std::string& username, bool correct, int timeBonus) {
    auto& scores = playerScores[roomId];
    auto& playerScore = scores[username];
    
    playerScore.totalAnswers++;
    if (correct) {
        playerScore.correctAnswers++;
        playerScore.score += 10 + timeBonus; // Base 10 points + time bonus
    }
    playerScore.lastAnswerTime = std::chrono::steady_clock::now();
}

std::string GameEngine::startGame(int roomId, const std::string& username, int questionCount) {
    // Check if user is room owner
    auto room = roomManager.getRoom(roomId);
    if (!room || room->getHostUsername() != username) {
        return "ERROR|Only room owner can start the game";
    }
    
    // Check if game is already active
    if (isGameActive(roomId)) {
        return "ERROR|Game is already in progress";
    }
    
    // Check if enough players
    auto players = roomManager.getRoomPlayers(roomId);
    if (players.size() < 1) {
        return "ERROR|Need at least 1 player to start";
    }
    
    // Get questions
    auto questions = questionManager.getRandomQuestions(questionCount);
    if (questions.empty()) {
        return "ERROR|No questions available";
    }
    
    // Initialize game state
    roomQuestions[roomId] = questions;
    roomPlayers[roomId] = players;
    
    auto& gameSession = gameSessions[roomId];
    gameSession.currentState = GameSession::WAITING;
    gameSession.totalQuestions = questions.size();
    // Set game timer
    gameSession.gameStartTime = std::chrono::steady_clock::now();
    gameSession.gameDurationSeconds = 90; // 1.5 minutes
    
    // Initialize player scores
    auto& scores = playerScores[roomId];
    scores.clear();
    for (const auto& player : players) {
        scores[player] = PlayerScore();
        scores[player].username = player;
        gameSession.playerQuestionIndex[player] = 0;
    }
    
    // Start the game
    startNewRound(roomId);
    
    std::ostringstream oss;
    oss << "GAME_STARTED|" << questionCount << " questions|" << players.size() << " players";
    return oss.str();
}

std::string GameEngine::endGame(int roomId, const std::string& username) {
    auto room = roomManager.getRoom(roomId);
    if (!room || room->getHostUsername() != username) {
        return "ERROR|Only room owner can end the game";
    }
    
    if (!isGameActive(roomId)) {
        return "ERROR|No active game to end";
    }
    
    endRound(roomId);
    
    // Get final leaderboard
    std::string leaderboard = getLeaderboard(roomId);
    
    // Cleanup
    cleanupRoom(roomId);
    
    return "GAME_ENDED|" + leaderboard;
}

std::string GameEngine::getCurrentQuestion(int roomId, const std::string& username) {
    if (!isGameActive(roomId)) {
        return "ERROR|No active game";
    }
    if (isGameTimerExpired(roomId)) {
        endRound(roomId);
        return "ERROR|Game timer expired|GAME_FINISHED";
    }
    if (!isPlayerInGame(roomId, username)) {
        return "ERROR|Player not in game";
    }
    auto& gameSession = gameSessions[roomId];
    auto& questions = roomQuestions[roomId];
    int playerIdx = 0;
    if (gameSession.playerQuestionIndex.count(username))
        playerIdx = gameSession.playerQuestionIndex[username];
    if (playerIdx >= static_cast<int>(questions.size())) {
        return "ERROR|No more questions|GAME_FINISHED";
    }
    const auto& question = questions[playerIdx];
    // Calculate remaining time
    auto now = std::chrono::steady_clock::now();
    int secondsLeft = gameSession.gameDurationSeconds - std::chrono::duration_cast<std::chrono::seconds>(now - gameSession.gameStartTime).count();
    if (secondsLeft < 0) secondsLeft = 0;
    std::ostringstream oss;
    oss << "QUESTION|" << (playerIdx + 1) << "/" << gameSession.totalQuestions
        << "|" << question.getQuestionText() << "|" << secondsLeft;
    for (size_t i = 0; i < question.getOptions().size(); ++i) {
        oss << "|" << (i + 1) << "." << question.getOptions()[i];
    }
    return oss.str();
}

std::string GameEngine::submitAnswer(int roomId, const std::string& username, int answerIndex) {
    if (!isGameActive(roomId)) {
        return "ERROR|No active game";
    }
    if (isGameTimerExpired(roomId)) {
        endRound(roomId);
        return "ERROR|Game timer expired|GAME_FINISHED";
    }
    if (!isPlayerInGame(roomId, username)) {
        return "ERROR|Player not in game";
    }
    auto& gameSession = gameSessions[roomId];
    auto& questions = roomQuestions[roomId];
    int& playerIdx = gameSession.playerQuestionIndex[username];
    if (playerIdx >= static_cast<int>(questions.size())) {
        return "ERROR|No more questions|GAME_FINISHED";
    }
    const auto& question = questions[playerIdx];
    // Validate answer index
    if (answerIndex < 1 || answerIndex > question.getOptionCount()) {
        return "ERROR|Invalid answer index";
    }
    // Check if already answered (optional: can add per-player answer tracking)
    auto& scores = playerScores[roomId];
    auto& playerScore = scores[username];
    auto now = std::chrono::steady_clock::now();
    auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - gameSession.questionStartTime);
    // Calculate time bonus (max 5 points for answering quickly)
    int timeBonus = 0;
    if (timeDiff.count() <= 10) { // Within 10 seconds
        timeBonus = 5 - (timeDiff.count() / 2);
    }
    // Check if answer is correct
    bool correct = question.isCorrectAnswer(answerIndex - 1);
    awardPoints(roomId, username, correct, timeBonus);
    playerIdx++;
    bool finished = (playerIdx >= static_cast<int>(questions.size()));
    std::ostringstream oss;
    oss << "ANSWER_RESULT|" << (correct ? "CORRECT" : "INCORRECT") 
        << "|" << (question.getCorrectAnswerIndex() + 1)
        << "|" << question.getCorrectAnswer() << "|" << playerScore.score;
    if (finished) {
        oss << "|GAME_FINISHED";
    }
    return oss.str();
}

std::string GameEngine::getGameInfo(int roomId, const std::string& /*username*/) {
    auto it = gameSessions.find(roomId);
    if (it == gameSessions.end()) {
        return "NO_GAME";
    }
    
    std::ostringstream oss;
    oss << "GAME_INFO|" << getGameStatus(roomId);
    
    // Add player count
    auto players = getActivePlayers(roomId);
    oss << "|Players:" << players.size();
    
    // Add current leaderboard
    oss << "|" << getLeaderboard(roomId);
    
    return oss.str();
}

std::string GameEngine::getLeaderboard(int roomId, const std::string& /*username*/) {
    return getLeaderboard(roomId);
}

bool GameEngine::isPlayerInGame(int roomId, const std::string& username) {
    auto it = roomPlayers.find(roomId);
    if (it == roomPlayers.end()) {
        return false;
    }
    
    return std::find(it->second.begin(), it->second.end(), username) != it->second.end();
}

bool GameEngine::canStartGame(int roomId, const std::string& username) {
    auto room = roomManager.getRoom(roomId);
    return room && room->getHostUsername() == username && !isGameActive(roomId);
}

int GameEngine::getPlayerCount(int roomId) {
    auto it = roomPlayers.find(roomId);
    return it != roomPlayers.end() ? it->second.size() : 0;
}

std::vector<std::string> GameEngine::getActivePlayers(int roomId) {
    auto it = roomPlayers.find(roomId);
    return it != roomPlayers.end() ? it->second : std::vector<std::string>();
}

void GameEngine::removePlayer(int roomId, const std::string& username) {
    // Remove from room players
    auto it = roomPlayers.find(roomId);
    if (it != roomPlayers.end()) {
        auto& players = it->second;
        players.erase(std::remove(players.begin(), players.end(), username), players.end());
    }
    
    // Remove from scores
    auto scoreIt = playerScores.find(roomId);
    if (scoreIt != playerScores.end()) {
        scoreIt->second.erase(username);
    }
    
    // If no players left, end the game
    if (getPlayerCount(roomId) == 0) {
        cleanupRoom(roomId);
    }
}

void GameEngine::cleanupRoom(int roomId) {
    gameSessions.erase(roomId);
    playerScores.erase(roomId);
    roomQuestions.erase(roomId);
    roomPlayers.erase(roomId);
} 

bool GameEngine::isGameTimerExpired(int roomId) {
    auto it = gameSessions.find(roomId);
    if (it == gameSessions.end()) return true;
    const auto& gameSession = it->second;
    if (gameSession.currentState != GameSession::PLAYING) return false;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - gameSession.gameStartTime).count();
    return elapsed >= gameSession.gameDurationSeconds;
} 