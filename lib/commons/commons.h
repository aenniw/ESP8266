#ifndef COMMONS_H
#define COMMONS_H

#include <stdlib.h>
#include <stdint.h>

void *checked_free(void *ptr);

class Service {
public:
    virtual void cycle_routine();
};

typedef enum {
    ANY, DIGITAL_IO
} DEVICE_TYPE;

class Device {
public:
    virtual uint8_t get_id() const;

    virtual DEVICE_TYPE get_type() const;
};

#endif //COMMONS_H
