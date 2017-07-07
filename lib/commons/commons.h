#ifndef COMMONS_H
#define COMMONS_H

#include <stdlib.h>
#include <stdint.h>

#define CONFIG_GLOBAL_JSON      "/json/config-global.json"

void *checked_free(void *ptr);

class Service {
public:
    virtual void cycle_routine() = 0;
};

typedef enum {
    ANY, DIGITAL_IO
} DEVICE_TYPE;

class Device {
public:
    virtual uint8_t get_id() const = 0;

    virtual DEVICE_TYPE get_type() const = 0;

    virtual void clean() = 0; // FIXME: remove and use only destructor

    //virtual ~Device() {};
};

#endif //COMMONS_H
