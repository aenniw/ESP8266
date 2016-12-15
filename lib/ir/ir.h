#ifndef WEMOS_D1_IR_H
#define WEMOS_D1_IR_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <Ticker.h>
#include <map>
#include <API.h>
#include <functional>

#define IR_SERVICE_NAME    "_ir_service_"

typedef std::function<void(void)> IRServiceFunction;

class IR : public ESP_Service {
protected:
    Ticker *ir_timer = NULL;
    IRrecv *ir_recv = NULL;
    std::map<unsigned long, IRServiceFunction> handlers_map;
    decode_results results;
private:
    void static timer_tick(IR *);

public:
    IR(const uint8_t port) : IR(port, 0) {}

    IR(const uint8_t, const uint32_t);

    void cycle_routine();

    const char *get_name() { return IR_SERVICE_NAME; };

    void add_handler(unsigned long key, IRServiceFunction);

    virtual ~IR() {
        if (ir_timer != NULL)
            delete ir_timer;
        delete ir_recv;
    }
};

#endif // WEMOS_D1_IR_H
