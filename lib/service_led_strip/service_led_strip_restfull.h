#ifndef ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_RESTFULL_H
#define ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_RESTFULL_H

#include <commons_json.h>
#include <service_rest.h>
#include <service_led_strip.h>

#if defined(RESTFULL_UI) || defined(RESTFULL_CALLS)

#define HTML_STRIP "/html/svc/l-strip.html"
#define JS_STRIP "/js/svc/l-strip.js"

class RestFullLedStripService : public LedStripService {
public:
    RestFullLedStripService(const LED_STRIP_TYPE t, const LED_STRIP_TRANSFER_MODE m, const uint16_t l,
                            RestService *web_service, const bool auth = true)
            : LedStripService(t, m, l) {
#ifdef RESTFULL_UI
        web_service->add_handler_file(HTML_STRIP, HTTP_ANY, RESP_HTML, HTML_STRIP
                ".gz", auth);
        web_service->add_handler_file(JS_STRIP, HTTP_ANY, RESP_JS, JS_STRIP
                ".gz", auth);
#endif
        web_service->add_handler("/led-strip/get-config", HTTP_GET, RESP_JSON, [this](String arg) -> String {
            String resp = "{ \"animation-type\" :";
            resp += get_mode();
            resp += ", \"length\" :";
            resp += get_len();
            resp += ", \"color\" :";
            resp += get_rgb();
            resp += ", \"brightness\" :";
            resp += get_brightness();
            resp += ", \"speed\" :";
            resp += 255 - get_delay();
            resp += ", \"mode\" :";
            resp += get_transfer_mode();
            resp += ", \"type\" :";
            resp += get_type();
            return resp + "}";
        }, auth);
        web_service->add_handler("/led-strip/get-animation-colors", HTTP_GET, RESP_JSON, [this](String arg) -> String {
            String resp = "{ \"animation-colors\" : [";
            uint8_t len = 0;
            const uint32_t *palette = get_animation_palette_rgb(&len);
            for (uint8_t i = 0; i < len; i++) {
                resp += palette[i];
                if (i + 1 < len) resp += ",";
            }
            return resp + "]}";
        }, auth);
        web_service->add_handler("/led-strip/set-animation-colors", HTTP_POST, RESP_JSON, [this](String arg) -> String {
            StaticJsonBuffer<512> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            if (json.containsKey("animation-colors") && json["animation-colors"].is<JsonArray>()) {
                JsonArray &array = json["animation-colors"].as<JsonArray>();
                uint8_t len = 0;
                for (auto &color:array) {
                    len++;
                }
                uint32_t colors[len];
                len = 0;
                for (auto &color:array) {
                    colors[len++] = color.as<uint32_t>();
                    Log::println("Color palette: %d", colors[len - 1]);
                }
                set_animation_palette_rgb(colors, len);
            }
            return JSON_RESP_NOK;
        }, auth);
        web_service->add_handler("/led-strip/set-mode", HTTP_POST, RESP_JSON, [this](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int16_t mode = parseJSON<int8_t>(json, "mode", -1);
            if (mode >= 0) {
                set_mode((LED_STRIP_ANIM_MODE) mode);
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, auth);
        web_service->add_handler("/led-strip/set-speed", HTTP_POST, RESP_JSON, [this](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int16_t speed = parseJSON<int16_t>(json, "speed", -1);
            if (speed >= 0) {
                set_delay((uint8_t) (255 - speed));
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, auth);
        web_service->add_handler("/led-strip/set-color", HTTP_POST, RESP_JSON, [this](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const uint32_t color = parseJSON<uint32_t>(json, "rgb", 0);
            set_rgb(color);
            return JSON_RESP_OK;
        }, auth);
        web_service->add_handler("/led-strip/set-brightness", HTTP_POST, RESP_JSON,
                                 [this](String arg) -> String {
                                     StaticJsonBuffer<100> jsonBuffer;
                                     JsonObject &json = jsonBuffer.parseObject(arg);
                                     if (!json.success())
                                         return JSON_RESP_NOK;
                                     const int8_t brightness = parseJSON<int8_t>(json, "brightness", -1);
                                     if (brightness >= 0) {
                                         set_brightness((uint8_t) brightness);
                                         return JSON_RESP_OK;
                                     }
                                     return JSON_RESP_NOK;
                                 }, auth);
    }
};

#endif

#endif //ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_RESTFULL_H
