#ifndef HID_SERVICE_H
#define HID_SERVICE_H

#include <Arduino.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <commons.h>

class HID : public Service {
private:
    static WebSocketsServer webSocket;

    HID(const char *user = NULL, const char *pass = NULL) {
        Serial.begin(9600);
        if (user && pass) {
            HID::webSocket.setAuthorization(user, pass);
        }
        HID::webSocket.begin();
        HID::webSocket.onEvent(webSocketEvent);
    }

    static void webSocketEvent(uint8_t, WStype_t, uint8_t *, size_t);

public:

    static HID *getInstance(const char *user = NULL, const char *pass = NULL) {
        static HID *service = new HID(user, pass);
        return service;
    }

    void cycle_routine();
};


#endif //HID_SERVICE_H
