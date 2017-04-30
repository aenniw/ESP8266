#include "devices.h"

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
        case RELAY:
            d = new Relay(p);
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

Relay::Relay(const uint8_t p) {
    pin = p;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
}

bool Relay::get_state() {
    return state;
}

void Relay::set_state(const bool flag) {
    digitalWrite(pin, flag ? 1 : 0);
}

uint8_t Relay::get_id() {
    return pin;
}

DEVICE_TYPE Relay::get_type() {
    return RELAY;
}