#ifndef COMMONS_H
#define COMMONS_H

#include <stdlib.h>

void *checked_free(void *ptr);

class Service {
public:
    virtual void cycle_routine();
};

class Device {
    // TODO: design API for sensors
};

#endif //COMMONS_H
