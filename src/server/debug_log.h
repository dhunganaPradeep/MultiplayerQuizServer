#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <string>
#include <fstream>
#include <ctime>

// Debug logging
extern std::ofstream debugLog;

void initDebugLog();
void debugLogMsg(const std::string& msg);
void closeDebugLog();

#endif 