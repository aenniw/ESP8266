#include "devices.h"

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