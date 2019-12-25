#include "service_hid.h"

WebSocketsServer HID::webSocket(9011);

static void handle_at(const uint8_t *payload, uint8_t len) {
    if (!payload || !len) {
        return;
    }
    const String msg((const char *) payload);
    if (msg.startsWith("AT+")) {
        const uint8_t type = (const uint8_t) msg.substring(3).toInt(),
                space_index = (uint8_t) msg.indexOf(' ');
        uint8_t msg_len = (uint8_t)(len - space_index + 1);
        if (type == 1) { // PRINT whole buffer
            Serial.write(msg_len + 1);                  // SEND LENGTH
            delay(50);
            Serial.write(type);                         // SEND TYPE
            for (uint8_t i = (uint8_t)(space_index + 1); i < len; i++) {
                Serial.write(*(payload + i));           // SEND PAYLOAD
                delay(50);
            }
            Serial.write('\n');
        } else { // PARSE rest of args to uint8_t
            msg_len = 2;
            for (unsigned int i = space_index; i < len; i = (unsigned int) msg.indexOf(' ', i + 1)) {
                if (*(payload + i - 1) != ' ') {
                    msg.substring(i).toInt();
                    msg_len++;
                }
            }
            Serial.write(msg_len);                             // SEND LENGTH
            delay(50);
            Serial.write(type);                                // SEND TYPE
            for (unsigned int i = space_index; i < len; i = (unsigned int) msg.indexOf(' ', i + 1)) {
                if (*(payload + i - 1) != ' ') {
                    Serial.write(msg.substring(i).toInt());    // SEND PAYLOAD
                    delay(50);
                }
            }
        }
    }
}

void HID::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t len) {
    switch (type) {
        case WStype_BIN:
            // TODO add binary handling for binary AT execution.
            break;
        case WStype_TEXT:
            handle_at(payload, (uint8_t) len);
            break;

    }
}

void HID::cycle_routine() {
    HID::webSocket.loop();
}