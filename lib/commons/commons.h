#ifndef COMMONS_H
#define COMMONS_H

#include <stdlib.h>

void *checked_free(void *ptr);

class ESP_Service {
public:
    virtual void cycle_routine();
};

class ESP_Sensor {
    // TODO: design API for sensors
};

#endif //COMMONS_H
