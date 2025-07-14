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

## Communication Protocol

The server and client communicate using a simple, human-readable, text-based protocol over TCP sockets.

- **Message Format:**  
  Each message is a single line of text, with fields separated by the `|` character.

- **Structure:**  
  ```
  COMMAND|param1|param2|...|paramN
  ```

- **Examples:**
  - Register: `REGISTER|username|password`
  - Login: `LOGIN|username|password`
  - Create Room: `CREATE_ROOM|username|room_name`
  - Join Room: `JOIN_ROOM|username|room_id`
  - Start Game: `START_GAME|username|room_id|num_questions`
  - Submit Answer: `SUBMIT_ANSWER|username|room_id|answer_index`
  - Quit: `QUIT`

- **Server Responses:**  
  The server responds with similar messages, e.g.:
  - `OK|Registration successful`
  - `ERROR|Room is full`
  - `GAME_RESPONSE|GAME_STARTED|5 questions|3 players`
  - `QUESTION|1/5|What is 2+2?|30|1. 3|2. 4|3. 5|4. 6`
  - `ANSWER_RESULT|CORRECT|2|10|GAME_FINISHED`
  - `LEADERBOARD|1.user1:30(3/3)|2.user2:20(2/3)|...`

#### Direct Game Messages
- **Question:** `QUESTION|question_number/total|question_text|time_left|1.option1|2.option2|...`
- **Answer Result:** `ANSWER_RESULT|CORRECT/INCORRECT|correct_answer|score|GAME_FINISHED?`

### Message Parsing
The code uses `std::getline()` with `|` delimiter to parse messages into a command and parameters vector.


# Authors
Vision Rijal - 201739
Pradip Dhungana - 201751
