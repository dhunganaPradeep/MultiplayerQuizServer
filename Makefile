# Makefile for Multiplayer Quiz Game System

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS =

# Directories
SRCDIR = src
SERVERDIR = $(SRCDIR)/server
CLIENTDIR = $(SRCDIR)/client
COMMONDIR = $(SRCDIR)/common

# Source files
SERVER_SOURCES = $(SERVERDIR)/main.cpp \
                 $(SERVERDIR)/authentication.cpp \
                 $(SERVERDIR)/room_manager.cpp \
                 $(SERVERDIR)/question_manager.cpp \
                 $(SERVERDIR)/game_engine.cpp \
                 $(SERVERDIR)/debug_log.cpp \
                 $(COMMONDIR)/protocol.cpp

CLIENT_SOURCES = $(CLIENTDIR)/main.cpp \
                 $(COMMONDIR)/protocol.cpp

# Output directory
BUILD_DIR = build

# Object files
SERVER_OBJECTS = \
	$(BUILD_DIR)/server_main.o \
	$(BUILD_DIR)/authentication.o \
	$(BUILD_DIR)/room_manager.o \
	$(BUILD_DIR)/question_manager.o \
	$(BUILD_DIR)/game_engine.o \
	$(BUILD_DIR)/debug_log.o \
	$(BUILD_DIR)/protocol.o
CLIENT_OBJECTS = \
	$(BUILD_DIR)/client_main.o \
	$(BUILD_DIR)/protocol.o

# Executables
SERVER_EXEC = $(BUILD_DIR)/server
CLIENT_EXEC = $(BUILD_DIR)/client

# Default target
all: $(BUILD_DIR) $(SERVER_EXEC) $(CLIENT_EXEC)

# Ensure build directory exists
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build server
$(SERVER_EXEC): $(SERVER_OBJECTS)
	$(CXX) $(SERVER_OBJECTS) -o $@
	@echo "Server built successfully: $@"

# Build client
$(CLIENT_EXEC): $(CLIENT_OBJECTS)
	$(CXX) $(CLIENT_OBJECTS) -o $@
	@echo "Client built successfully: $@"

# Compile object files into build dir
$(BUILD_DIR)/server_main.o: $(SERVERDIR)/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/client_main.o: $(CLIENTDIR)/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/authentication.o: $(SERVERDIR)/authentication.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/room_manager.o: $(SERVERDIR)/room_manager.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/question_manager.o: $(SERVERDIR)/question_manager.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/game_engine.o: $(SERVERDIR)/game_engine.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/debug_log.o: $(SERVERDIR)/debug_log.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/protocol.o: $(COMMONDIR)/protocol.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build files"

# Clean and rebuild
rebuild: clean all

# Test the system
test: all
	@echo "To test:"
	@echo "  1. In one terminal, run: ./$(SERVER_EXEC)"
	@echo "  2. In another terminal, run: ./$(CLIENT_EXEC)"
	@echo "(Or run both in background with ./$(SERVER_EXEC) & ./$(CLIENT_EXEC) &)"

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build server and client (default)"
	@echo "  server   - Build only server"
	@echo "  client   - Build only client"
	@echo "  clean    - Remove all build files"
	@echo "  rebuild  - Clean and rebuild everything"
	@echo "  test     - Build and run server/client in separate windows"
	@echo "  help     - Show this help message"

# Phony targets
.PHONY: all clean rebuild test help 