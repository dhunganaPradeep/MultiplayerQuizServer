#include "debug_log.h"
#include <iostream>

// Debug logging
std::ofstream debugLog;

void initDebugLog() {
    debugLog.open("server_debug.log", std::ios::app);
    if (debugLog.is_open()) {
        time_t now = time(0);
        char* dt = ctime(&now);
        debugLog << "\n=== Server Debug Log Started: " << dt;
        debugLog.flush();
    }
}

void debugLogMsg(const std::string& msg) {
    if (debugLog.is_open()) {
        debugLog << "[DEBUG] " << msg << std::endl;
        debugLog.flush();
    }
}

void closeDebugLog() {
    if (debugLog.is_open()) {
        debugLog.close();
    }
} 