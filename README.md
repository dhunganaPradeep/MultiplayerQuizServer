---

# Multiplayer Quiz Game (Linux, Makefile Build)

This project is a command-line, network-based multiplayer quiz game written in C++. It features a server and client, using low-level POSIX sockets for communication.

---

## Project Structure

```
MultiplayerQuizServer/
│
├── src/
│   ├── server/         # Server-side source code (main.cpp, game logic, room management, etc.)
│   ├── client/         # Client-side source code (main.cpp)
│   └── common/         # Shared code (protocol, data structures)
│
├── data/
│   ├── users.txt       # Persistent user data (auto-created if missing)
│   └── questions.txt   # Persistent question data (auto-created if missing)
│
├── build/              # All build artifacts (.o files, executables) go here
│
├── Makefile            # Linux build system (use this!)
└── README.md           # This file
```

---

## Building the Project (Linux)

1. **Install build tools** (if not already installed):
   ```sh
   sudo pacman -S base-devel   # Arch Linux
   # or
   sudo apt install build-essential   # Debian/Ubuntu
   ```

2. **Build the project:**
   ```sh
   make clean
   make
   ```

   - All object files and executables will be placed in the `build/` directory.
   - The server executable: `build/server`
   - The client executable: `build/client`

3. **Run the server:**
   ```sh
   ./build/server
   ```

4. **Run the client (in another terminal):**
   ```sh
   ./build/client
   ```

---

## Networking Implementation

The game uses **POSIX (BSD) sockets** for network communication, implemented in C++ with the following technical specifications:

### Socket Configuration
- **Protocol:** TCP (SOCK_STREAM) for reliable, ordered data transmission
- **Address Family:** IPv4 (AF_INET)
- **Server Port:** 8080 (configurable in `src/common/game_state.h`)
- **Server Address:** 127.0.0.1 (localhost)

### Server Architecture
- **I/O Model:** Non-blocking sockets with `select()` multiplexing
- **Concurrency:** Single-threaded event loop using `fd_set` for socket monitoring
- **Socket Management:**
  - Main listening socket (non-blocking)
  - Client sockets (non-blocking)
  - Dynamic client session tracking

### Client Architecture
- **Connection:** Single TCP connection to server
- **Blocking I/O:** Synchronous send/receive operations
- **Session Management:** Maintains connection throughout game session

### Key Networking Features
- **Non-blocking Server:** Uses `fcntl()` with `O_NONBLOCK` flag
- **Select-based Multiplexing:** Handles multiple clients efficiently
- **Robust Message Parsing:** Handles partial messages and buffering
- **Connection Cleanup:** Automatic cleanup of disconnected clients
- **Error Handling:** Comprehensive error checking for socket operations

### Socket Operations
- **Server:** `socket()` → `bind()` → `listen()` → `select()` → `accept()` → `recv()`/`send()`
- **Client:** `socket()` → `connect()` → `send()`/`recv()`

---

## Communication Protocol

The server and client communicate using a simple, human-readable, text-based protocol over TCP sockets. All messages end with a newline character (`\n`).

### Message Format
Each message is a single line of text, with fields separated by the `|` character.

**Structure:** `COMMAND|param1|param2|...|paramN`

### Client Commands

#### Authentication
- **Register:** `REGISTER|username|password`
- **Login:** `LOGIN|username|password`
- **Quit:** `QUIT`

#### Room Management
- **Create Room:** `CREATE_ROOM|username|room_name`
- **Browse Rooms:** `BROWSE_ROOMS`
- **Join Room:** `JOIN_ROOM|username|room_id`

#### Game Actions
- **Start Game:** `START_GAME|username|room_id|num_questions`
- **Get Current Question:** `GET_CURRENT_QUESTION|username|room_id`
- **Submit Answer:** `SUBMIT_ANSWER|username|room_id|answer_index`
- **Get Game Info:** `GET_GAME_INFO|username|room_id`
- **Get Leaderboard:** `GET_LEADERBOARD|username|room_id`
- **End Game:** `END_GAME|username|room_id`

### Server Responses

#### Success Responses
- **OK:** `OK|message` (e.g., `OK|Registration successful`)
- **Room List:** `ROOM_LIST|room_id1|room_name1|room_id2|room_name2|...`

#### Error Responses
- **ERROR:** `ERROR|error_message` (e.g., `ERROR|Room is full`)

#### Game Responses
- **Game Response:** `GAME_RESPONSE|payload`
  - Game Started: `GAME_RESPONSE|GAME_STARTED|5 questions|3 players`
  - Question: `GAME_RESPONSE|QUESTION|1/5|What is 2+2?|30|1. 3|2. 4|3. 5|4. 6`
  - Answer Result: `GAME_RESPONSE|ANSWER_RESULT|CORRECT|2|10`
  - Game Info: `GAME_RESPONSE|GAME_INFO|PLAYING|1/5|Players:3|Leaderboard...`
  - Leaderboard: `GAME_RESPONSE|1.user1:30(3/3)|2.user2:20(2/3)|...`

#### Direct Game Messages
- **Question:** `QUESTION|question_number/total|question_text|time_left|1.option1|2.option2|...`
- **Answer Result:** `ANSWER_RESULT|CORRECT/INCORRECT|correct_answer|score|GAME_FINISHED?`

### Message Parsing
The code uses `std::getline()` with `|` delimiter to parse messages into a command and parameters vector.

---


---
