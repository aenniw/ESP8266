#include "ir.h"


IR::IR(const uint8_t port, const uint32_t ir_check) {

    if (ir_check != 0) {
        ir_timer = new Ticker();
        ir_timer->attach_ms(ir_check, timer_tick, this);
    }
    ir_recv = new IRrecv(port);
    ir_recv->enableIRIn();
}

void IR::cycle_routine() {
    if (ir_recv->decode(&results)) {
#ifdef __DEBUG__
        Serial.println("Recieved");
        Serial.println(results.value, HEX);
#endif
        IRServiceFunction handler = handlers_map[results.value];
        if (handler != NULL)
            handler();
        ir_recv->resume(); // Receive the next value
    }
}

void IR::add_handler(unsigned long key, IRServiceFunction function) {
    handlers_map[key] = function;
}

void IR::timer_tick(IR *ir_service) {
    ir_service->cycle_routine();
}