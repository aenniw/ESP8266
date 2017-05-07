#include "devices.h"

static std::map<uint8_t, Device *> devices;
static const uint8_t BOARD_PINS[] = {
#ifdef  ARDUINO_ESP8266_ESP01
        0, 2, 3, 1
#else
        D0, D1, D2, D3, D4, D5, D6, D7, D8, RX, TX
#endif
};
static const size_t BOARD_PINS_LEN = sizeof(BOARD_PINS);


void Devices::get_devices(const DEVICE_TYPE t, std::list<Device *> *d) {
    if (d == NULL) {
        return;
    }
    for (std::map<uint8_t, Device *>::iterator i = devices.begin();
         i != devices.end(); i++) {
        if (t == ANY || (*i).second->get_type() == t) {
            d->push_back((*i).second);
        }
    }
}

const uint8_t *Devices::get_pins(size_t *len) {
    (*len) = BOARD_PINS_LEN;
    return BOARD_PINS;
}

const bool Devices::valid_pin(const uint8_t p) {
    for (uint32_t i = 0; i < BOARD_PINS_LEN; i++) {
        if (p == BOARD_PINS[i]) {
            return true;
        }
    }
    return false;
}

Device *Devices::put(const uint8_t p, const DEVICE_TYPE t) {
    if (!Devices::valid_pin(p) || Devices::get(p) != NULL) {
        return NULL;
    }
    Device *d = NULL;
    switch (t) {
        case DIGITAL_IO:
            d = new DigitalIO(p);
            break;
        default:
            return NULL;
    }
    devices[p] = d;
    return d;
}

bool Devices::remove(const uint8_t p) {
    const Device *d = get(p);
    if (d == NULL) {
        return false;
    }
    delete d;
    devices.erase(p);
    return true;
}

Device *Devices::get(const uint8_t p) {
    const std::map<uint8_t, Device *>::iterator iter = devices.find(p);
    if (iter == devices.end()) {
        return NULL;
    }
    return devices[p];
}

DigitalIO::DigitalIO(const uint8_t p) {
    pin = p;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
}

bool DigitalIO::get_state() {
    return state;
}

void DigitalIO::set_state(const bool flag) {
    state = flag ? 1 : 0;
    digitalWrite(pin, state);
}

uint8_t DigitalIO::get_id() const {
    return pin;
}

DEVICE_TYPE DigitalIO::get_type() const {
    return DIGITAL_IO;
}