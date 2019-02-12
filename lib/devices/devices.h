#ifndef ESP8266_PROJECTS_ROOT_DEVICES_H
#define ESP8266_PROJECTS_ROOT_DEVICES_H

#include <map>
#include <list>
#include <Arduino.h>
#include <commons.h>
#include <file_system.h>

class Devices {
private:
    static const bool valid_pin(const uint8_t);

    static Device *put(Device *);

public:
    static Device *put(const uint8_t, const DEVICE_TYPE);

    static Device *get(const uint8_t);

    static bool remove(const uint8_t);

    static const uint8_t *get_pins(size_t *);

    static void get_devices(const DEVICE_TYPE, std::list<Device *> *d);

    static uint8_t parse_devices(const char *, const DEVICE_TYPE);
};

class DigitalIO : public Device {
private:
    uint8_t pin, state = 0;

public:
    DigitalIO(const uint8_t, const bool = false, const uint8_t = OUTPUT);

    bool get_state();

    void set_state(const bool);

    uint8_t get_id() const override;

    DEVICE_TYPE get_type() const override;

    ~DigitalIO() override;
};

#endif //ESP8266_PROJECTS_ROOT_DEVICES_H
