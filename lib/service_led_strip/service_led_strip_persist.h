#ifndef ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_PERSIST_H
#define ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_PERSIST_H

#include <file_system.h>
#include <service_led_strip_restfull.h>

class PersistentLedStripService : public RestFullLedStripService {
public:
    PersistentLedStripService(RestService *web_service) : RestFullLedStripService(
            (LED_STRIP_TYPE) ConfigJSON::get<uint8_t>(CONFIG_LS_JSON, {"type"}),
            (LED_STRIP_TRANSFER_MODE) ConfigJSON::get<uint8_t>(CONFIG_LS_JSON, {"transfer-mode"}),
            ConfigJSON::get<uint16_t>(CONFIG_LS_JSON, {"length"}),
            web_service) {
        set_color((uint32_t) ConfigJSON::get<uint32>(CONFIG_LS_JSON, {"color"}));
        set_mode((LED_STRIP_ANIM_MODE) ConfigJSON::get<uint8>(CONFIG_LS_JSON, {"mode"}));
    }
};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_PERSIST_H
