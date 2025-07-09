#ifndef ROOM_MANAGER_H
#define ROOM_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include "../common/room.h"
#include "../common/user.h"

enum class JoinRoomResult {
    SUCCESS,
    ROOM_NOT_FOUND,
    ROOM_FULL,
    GAME_IN_PROGRESS,
    USER_ALREADY_IN_ROOM,
    USER_ALREADY_IN_THIS_ROOM
};

class RoomManager {
private:
    std::map<int, Room> rooms;  // roomId -> Room
    int nextRoomId;  // Auto-incrementing room ID

public:
    RoomManager();
    ~RoomManager();

    // Room creation and management
    int createRoom(const std::string& roomName, const std::string& hostUsername);
    JoinRoomResult joinRoom(int roomId, const std::string& username);
    bool leaveRoom(int roomId, const std::string& username);
    bool deleteRoom(int roomId);
    
    // Room queries
    Room* getRoom(int roomId);
    std::vector<Room> getAllRooms() const;
    std::vector<Room> getAvailableRooms() const;  // Rooms that are waiting for players
    bool roomExists(int roomId) const;
    bool isUserInRoom(const std::string& username) const;
    int getUserRoomId(const std::string& username) const;
    
    // Room state management
    bool startGame(int roomId, const std::string& hostUsername);
    bool endGame(int roomId);
    bool isGameInProgress(int roomId) const;
    
    // Player management
    std::vector<std::string> getRoomPlayers(int roomId) const;
    int getRoomPlayerCount(int roomId) const;
    bool isPlayerInRoom(int roomId, const std::string& username) const;
    
    // Utility
    int getRoomCount() const { return static_cast<int>(rooms.size()); }
    void clearRooms() { rooms.clear(); nextRoomId = 1; }
    
    // Room ID generation
    int generateRoomId() { return nextRoomId++; }
};

#endif // ROOM_MANAGER_H 