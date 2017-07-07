#ifndef WEMOS_D1_IR_H
#define WEMOS_D1_IR_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <map>
#include <commons.h>
#include <functional>
#include <service_log.h>

typedef std::function<void(void)> IRServiceFunction;

class IRService : public Service {
protected:
    std::map<unsigned long, IRServiceFunction> handlers_map;
    IRrecv *ir_recv = NULL;

public:
    IRService(const uint8_t);

    void cycle_routine();

    void add_handler(unsigned long key, IRServiceFunction);

    virtual ~IRService();
};

#endif // WEMOS_D1_IR_H
