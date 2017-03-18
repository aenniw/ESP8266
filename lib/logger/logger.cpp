#include "logger.h"


void Log::init() {
#ifdef __DEBUG__
    Serial.begin(115200);
    Serial.println();
#endif
}

void Log::print(const char *format, ...) {
#ifdef __DEBUG__
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    Serial.print(buffer);
    if (buffer != temp) {
        delete[] buffer;
    }
#endif
}

void Log::println(const char *format, ...) {
#ifdef __DEBUG__
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    Serial.println(buffer);
    if (buffer != temp) {
        delete[] buffer;
    }
#endif
}

void Log::println(const String &msg) {
#ifdef __DEBUG__
    Serial.println(msg);
#endif
}