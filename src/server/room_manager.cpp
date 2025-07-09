#include "room_manager.h"
#include <algorithm>
#include <iostream>
#include "../common/game_state.h"

RoomManager::RoomManager() : nextRoomId(1) {
    std::cout << "Room manager initialized." << std::endl;
}

RoomManager::~RoomManager() {
    std::cout << "Room manager shutting down." << std::endl;
}

int RoomManager::createRoom(const std::string& roomName, const std::string& hostUsername) {
    // Validate room name
    if (roomName.empty() || roomName.length() > 50) {
        return -1;
    }
    
    // Check if user is already in a room
    if (isUserInRoom(hostUsername)) {
        return -1;
    }
    
    // Create new room
    int roomId = generateRoomId();
    Room newRoom(roomId, roomName, hostUsername);
    newRoom.addPlayer(hostUsername);
    rooms[roomId] = newRoom;
    
    std::cout << "Room created: " << roomName << " (ID: " << roomId << ") by " << hostUsername << std::endl;
    return roomId;
}

JoinRoomResult RoomManager::joinRoom(int roomId, const std::string& username) {
    auto it = rooms.find(roomId);
    if (it == rooms.end()) {
        return JoinRoomResult::ROOM_NOT_FOUND;
    }
    Room& room = it->second;
    if (room.getPlayerCount() >= GameConstants::MAX_PLAYERS_PER_ROOM) {
        return JoinRoomResult::ROOM_FULL;
    }
    if (room.isGameInProgress()) {
        return JoinRoomResult::GAME_IN_PROGRESS;
    }
    if (isUserInRoom(username)) {
        return JoinRoomResult::USER_ALREADY_IN_ROOM;
    }
    if (room.hasPlayer(username)) {
        return JoinRoomResult::USER_ALREADY_IN_THIS_ROOM;
    }
    room.addPlayer(username);
    std::cout << "User " << username << " joined room " << roomId << std::endl;
    return JoinRoomResult::SUCCESS;
}

bool RoomManager::leaveRoom(int roomId, const std::string& username) {
    auto it = rooms.find(roomId);
    if (it == rooms.end()) {
        return false;
    }
    
    Room& room = it->second;
    
    if (!room.hasPlayer(username)) {
        return false;
    }
    
    room.removePlayer(username);
    
    // If room is empty, delete it
    if (room.getPlayerCount() == 0) {
        rooms.erase(it);
        std::cout << "Room " << roomId << " deleted (empty)." << std::endl;
    } else {
        // If host left, assign new host
        if (room.getHostUsername() == username) {
            if (room.getPlayerCount() > 0) {
                room.hostUsername = room.getPlayers()[0];
                std::cout << "New host for room " << roomId << ": " << room.getHostUsername() << std::endl;
            }
        }
        std::cout << "User " << username << " left room " << roomId << std::endl;
    }
    
    return true;
}

bool RoomManager::deleteRoom(int roomId) {
    auto it = rooms.find(roomId);
    if (it != rooms.end()) {
        rooms.erase(it);
        std::cout << "Room " << roomId << " deleted." << std::endl;
        return true;
    }
    return false;
}

Room* RoomManager::getRoom(int roomId) {
    auto it = rooms.find(roomId);
    if (it != rooms.end()) {
        return &(it->second);
    }
    return nullptr;
}

std::vector<Room> RoomManager::getAllRooms() const {
    std::vector<Room> roomList;
    for (const auto& pair : rooms) {
        roomList.push_back(pair.second);
    }
    return roomList;
}

std::vector<Room> RoomManager::getAvailableRooms() const {
    std::vector<Room> availableRooms;
    for (const auto& pair : rooms) {
        const Room& room = pair.second;
        if (room.isWaiting() && room.getPlayerCount() < GameConstants::MAX_PLAYERS_PER_ROOM) {
            availableRooms.push_back(room);
        }
    }
    return availableRooms;
}

bool RoomManager::roomExists(int roomId) const {
    return rooms.find(roomId) != rooms.end();
}

bool RoomManager::isUserInRoom(const std::string& username) const {
    for (const auto& pair : rooms) {
        if (pair.second.hasPlayer(username)) {
            return true;
        }
    }
    return false;
}

int RoomManager::getUserRoomId(const std::string& username) const {
    for (const auto& pair : rooms) {
        if (pair.second.hasPlayer(username)) {
            return pair.first;
        }
    }
    return -1;
}

bool RoomManager::startGame(int roomId, const std::string& hostUsername) {
    auto it = rooms.find(roomId);
    if (it == rooms.end()) {
        return false;
    }
    
    Room& room = it->second;
    
    // Check if user is the host
    if (room.getHostUsername() != hostUsername) {
        return false;
    }
    
    // Check if game is already in progress
    if (room.isGameInProgress()) {
        return false;
    }
    
    // Check if enough players
    if (room.getPlayerCount() < GameConstants::MIN_PLAYERS_TO_START) {
        return false;
    }
    
    room.setGameState(GameState::PLAYING);
    room.setCurrentQuestionIndex(0);
    
    std::cout << "Game started in room " << roomId << std::endl;
    return true;
}

bool RoomManager::endGame(int roomId) {
    auto it = rooms.find(roomId);
    if (it == rooms.end()) {
        return false;
    }
    
    Room& room = it->second;
    room.setGameState(GameState::FINISHED);
    
    std::cout << "Game ended in room " << roomId << std::endl;
    return true;
}

bool RoomManager::isGameInProgress(int roomId) const {
    auto it = rooms.find(roomId);
    if (it != rooms.end()) {
        return it->second.isGameInProgress();
    }
    return false;
}

std::vector<std::string> RoomManager::getRoomPlayers(int roomId) const {
    auto it = rooms.find(roomId);
    if (it != rooms.end()) {
        return it->second.getPlayers();
    }
    return std::vector<std::string>();
}

int RoomManager::getRoomPlayerCount(int roomId) const {
    auto it = rooms.find(roomId);
    if (it != rooms.end()) {
        return it->second.getPlayerCount();
    }
    return 0;
}

bool RoomManager::isPlayerInRoom(int roomId, const std::string& username) const {
    auto it = rooms.find(roomId);
    if (it != rooms.end()) {
        return it->second.hasPlayer(username);
    }
    return false;
} 