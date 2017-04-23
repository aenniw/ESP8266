#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <stdarg.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <commons.h>

class Log : public ESP_Service {
private:
    static WebSocketsServer webSocket;
public:
    static void init();

    static Log *getInstance() {
        static Log *logger = new Log();
        return logger;
    }

    static void webSocketEvent(uint8_t, WStype_t, uint8_t *, size_t);

    static void print(const char *format, ...);

    static void println(const char *format, ...);

    static void println(const String &msg);

    void cycle_routine();
};


#endif //LOGGER_H
