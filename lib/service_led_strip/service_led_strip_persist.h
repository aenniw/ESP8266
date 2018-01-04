#ifndef ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_PERSIST_H
#define ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_PERSIST_H

#include <file_system.h>
#include <service_led_strip_restfull.h>

#define CONFIG_LS_JSON "/json/config-ls.json"

class PersistentLedStripService : public RestFullLedStripService {
public:
    PersistentLedStripService(RestService *web_service, const bool auth = true) : RestFullLedStripService(
            (LED_STRIP_TYPE) ConfigJSON::get<uint8_t>(CONFIG_LS_JSON, {"type"}),
            (LED_STRIP_TRANSFER_MODE) ConfigJSON::get<uint8_t>(CONFIG_LS_JSON, {"transfer-mode"}),
            ConfigJSON::get<uint16_t>(CONFIG_LS_JSON, {"length"}),
            web_service, auth) {
        RestFullLedStripService::set_rgb(ConfigJSON::get<uint32>(CONFIG_LS_JSON, {"color"}));
        RestFullLedStripService::set_delay(ConfigJSON::get<uint8_t>(CONFIG_LS_JSON, {"delay"}));
        const uint8_t len = (uint8_t) ConfigJSON::get_array_len(CONFIG_LS_JSON, {"palette"});
        if (len) {
            uint32_t *palette = ConfigJSON::get_array<uint32_t>(CONFIG_LS_JSON, {"palette"});
            RestFullLedStripService::set_animation_palette_rgb(palette, len);
            delete palette;
        }
        RestFullLedStripService::set_mode((LED_STRIP_ANIM_MODE) ConfigJSON::get<uint8_t>(CONFIG_LS_JSON, {"mode"}));

        web_service->add_handler("/led-strip/set-config", HTTP_POST, RESP_JSON, [this](String arg) -> String {
            StaticJsonBuffer<150> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int32_t len = parseJSON<int8_t>(json, "length", -1);
            const int8_t type = parseJSON<int8_t>(json, "type", -1);
            const int8_t mode = parseJSON<int8_t>(json, "mode", -1);
            if (len > 0 && type >= 0 && mode >= 0) {
                ConfigJSON::set<uint16_t>(CONFIG_LS_JSON, {"length"}, (uint16_t) len);
                ConfigJSON::set<uint8_t>(CONFIG_LS_JSON, {"type"}, (uint8_t) type);
                ConfigJSON::set<uint8_t>(CONFIG_LS_JSON, {"transfer-mode"}, (uint8_t) mode);
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, true);
    }

    void set_mode(const LED_STRIP_ANIM_MODE m) override {
        RestFullLedStripService::set_mode(m);
        ConfigJSON::set<uint8_t>(CONFIG_LS_JSON, {"mode"}, (uint8_t) m);
    }

    void set_rgb(const uint32_t color) override {
        RestFullLedStripService::set_rgb(color);
        ConfigJSON::set<uint32_t>(CONFIG_LS_JSON, {"color"}, color);
    }

    void set_brightness(const uint8_t b) override {
        RestFullLedStripService::set_brightness(b);
        ConfigJSON::set<uint32_t>(CONFIG_LS_JSON, {"color"}, get_rgb());
    }

    void set_delay(const uint8_t d) override {
        RestFullLedStripService::set_delay(d);
        ConfigJSON::set<uint8_t>(CONFIG_LS_JSON, {"delay"}, d);
    }

    void set_animation_palette_rgb(const uint32_t *p, const uint8_t len) override {
        RestFullLedStripService::set_animation_palette_rgb(p, len);
        ConfigJSON::clear_array(CONFIG_LS_JSON, {"palette"});
        for (uint8_t i = 0; i < len; i++) {
            ConfigJSON::add_to_array<uint32_t>(CONFIG_LS_JSON, {"palette"}, p[i]);
        }
    }
};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_PERSIST_H
