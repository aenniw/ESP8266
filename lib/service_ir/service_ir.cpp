#include "service_ir.h"

IRService::IRService(const uint8_t port) {
    ir_recv = new IRrecv(port);
    ir_recv->enableIRIn();
}

void IRService::cycle_routine() {
    decode_results results;
    if (ir_recv->decode(&results)) {
        Log::println("Recieved");
        Log::println("%#X", results.value);
        IRServiceFunction handler = handlers_map[results.value];
        if (handler != NULL)
            handler();
        ir_recv->resume();
    }
}

void IRService::add_handler(unsigned long key, IRServiceFunction function) {
    handlers_map[key] = function;
}

IRService::~IRService() {
    delete ir_recv;
}