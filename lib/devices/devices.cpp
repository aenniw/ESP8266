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
    return put(d);
}

uint8_t Devices::parse_devices(const char *file, const DEVICE_TYPE t) {
    const char *type = t == DIGITAL_IO ? "digital-io" : "analog-io";
    uint8_t parsed_device = 0;
    File configFile = SPIFFS.open(file, "r");
    if (!configFile) {
        Log::println("No file to parse io from.");
        return 0;
    }
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    configFile.close();
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (json.success() && json.containsKey(type)) {
        JsonObject &ios = json[type].as<JsonObject>();
        for (JsonObject::iterator it = ios.begin(); it != ios.end(); ++it) {
            if (!it->key || !it->value) continue;
            JsonObject &device = it->value.as<JsonObject &>();
            Device *d = NULL;
            switch (t) {
                case DIGITAL_IO:
                    const bool state = device["state"].as<bool>();
                    d = (Device *)
                            new DigitalIO((uint8_t) strtol(it->key, '\0', 10), state);
                    break;
            }
            if (Devices::put(d)) {
                parsed_device++;
            }
        }
    }
    return parsed_device;
}

Device *Devices::put(Device *d) {
    if (d) {
        devices[d->get_id()] = d;
    }
    return d;
}

bool Devices::remove(const uint8_t p) {
    Device *d = get(p);
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

DigitalIO::DigitalIO(const uint8_t p, const bool s, const uint8_t m) {
    pinMode((pin = p), m);
    set_state(s);
}

bool DigitalIO::get_state() {
    return state;
}

void DigitalIO::set_state(const bool flag) {
    char id[4] = {'\0'};
    digitalWrite(pin, (state = (uint8_t) (flag ? 1 : 0)));
    ConfigJSON::set<bool>(CONFIG_IO_JSON, {"digital-io", itoa(pin, id, 10), "state"}, flag);
}

uint8_t DigitalIO::get_id() const {
    return pin;
}

DEVICE_TYPE DigitalIO::get_type() const {
    return DIGITAL_IO;
}

DigitalIO::~DigitalIO() {
    char id[4] = {'\0'};
    ConfigJSON::del(CONFIG_IO_JSON, {"digital-io", itoa(pin, id, 10)});
}