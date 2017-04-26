#ifndef COMMONS_H
#define COMMONS_H

#include <stdlib.h>
#include <stdint.h>

void *checked_free(void *ptr);

class Service {
public:
    virtual void cycle_routine();
};

class Device {
    // TODO: design API for sensors
    virtual uint8_t get_id();
};

#endif //COMMONS_H
