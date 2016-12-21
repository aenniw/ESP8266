#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <stdarg.h>

// TODO: add support for web-socket logging
class Log {
public:
    static void init();

    static void println(const char *format, ...);

    static void println(const String &msg);
};


#endif //LOGGER_H
