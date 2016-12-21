#include "logger.h"


void Log::init() {
#ifdef __DEBUG__
    Serial.begin(115200);
    Serial.println();
#endif
}

void Log::println(const char *format, ...) {
#ifdef __DEBUG__
    va_list args;
    va_start(args, format);
    Serial.printf(format, args);
    Serial.println();
    va_end(args);
#endif
}

void Log::println(const String &msg) {
#ifdef __DEBUG__
    Serial.println(msg);
#endif
}