#ifndef UTILS_HPP
#define UTILS_HPP

#ifdef _DEBUG
#include <sstream>
#define LOG_MESSAGE(msg) { \
    std::stringstream msgStr; \
    msgStr << "[LOG]: " << msg << "\n"; \
    LogError(msgStr.str().c_str()); \
}

#define LOG_MESSAGE_ASSERT(x, msg) \
    if(!x) { \
    std::stringstream msgStr; \
    msgStr << "[!ASSERT!]: " << msg << "\n"; \
    LogError(msgStr.str().c_str()); \
    __debugbreak(); \
    }
#else
#define LOG_MESSAGE(msg)
#define LOG_MESSAGE_ASSERT(x, msg)
#endif


void LogError(const char *msg);
void ClearWindow(float r, float g, float b);
void DrawRectangle(float x, float y, float w, float h, float r, float g, float b);
#endif
