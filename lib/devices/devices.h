#ifndef ESP8266_PROJECTS_ROOT_DEVICES_H
#define ESP8266_PROJECTS_ROOT_DEVICES_H

#include <Arduino.h>
#include <commons.h>

class Relay : public Device {
private:
    uint8_t pin, state = 0;
public:
    Relay(const uint8_t);

    bool get_state();

    void set_state(const bool);

    uint8_t get_id();
};

#endif //ESP8266_PROJECTS_ROOT_DEVICES_H
