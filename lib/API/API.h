#ifndef WEMOS_D1_API_H
#define WEMOS_D1_API_H

#include <map>

class ESP_Service {
protected:
public:
    virtual const char *get_name();

    virtual void cycle_routine();
};


#endif //WEMOS_D1_API_H
