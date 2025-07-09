#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <string>

// Game constants
namespace GameConstants {
    const int MAX_PLAYERS_PER_ROOM = 10;
    const int MIN_PLAYERS_TO_START = 2;
    const int POINTS_PER_CORRECT_ANSWER = 10;
    const int QUESTION_TIME_LIMIT_SECONDS = 30;
    const int MAX_QUESTIONS_PER_GAME = 10;
    const int DEFAULT_PORT = 8080;
    const std::string DEFAULT_HOST = "127.0.0.1";
}

// Game events/messages
namespace GameEvents {
    const std::string PLAYER_JOINED = "PLAYER_JOINED";
    const std::string PLAYER_LEFT = "PLAYER_LEFT";
    const std::string GAME_STARTED = "GAME_STARTED";
    const std::string GAME_FINISHED = "GAME_FINISHED";
    const std::string QUESTION_ANSWERED = "QUESTION_ANSWERED";
    const std::string SCORE_UPDATED = "SCORE_UPDATED";
}

// Error messages
namespace ErrorMessages {
    const std::string ROOM_FULL = "Room is full";
    const std::string ROOM_NOT_FOUND = "Room not found";
    const std::string USER_NOT_FOUND = "User not found";
    const std::string INVALID_CREDENTIALS = "Invalid username or password";
    const std::string USERNAME_TAKEN = "Username already taken";
    const std::string GAME_ALREADY_STARTED = "Game already in progress";
    const std::string NOT_ENOUGH_PLAYERS = "Not enough players to start";
    const std::string NOT_HOST = "Only the host can perform this action";
}

// Success messages
namespace SuccessMessages {
    const std::string REGISTRATION_SUCCESS = "Registration successful";
    const std::string LOGIN_SUCCESS = "Login successful";
    const std::string ROOM_CREATED = "Room created successfully";
    const std::string ROOM_JOINED = "Joined room successfully";
    const std::string GAME_STARTED = "Game started successfully";
    const std::string ANSWER_CORRECT = "Correct answer!";
    const std::string ANSWER_INCORRECT = "Incorrect answer";
}

// Utility functions
namespace GameUtils {
    // Convert game state enum to string
    std::string gameStateToString(int state);
    
    // Validate username format
    bool isValidUsername(const std::string& username);
    
    // Validate room name format
    bool isValidRoomName(const std::string& roomName);
    
    // Generate unique room ID
    int generateRoomId();
    
    // Generate unique question ID
    int generateQuestionId();
}

#endif // GAME_STATE_H 