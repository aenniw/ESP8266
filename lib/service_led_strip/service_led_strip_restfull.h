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
                            RestService *web_service)
            : LedStripService(t, m, l) {
#ifdef RESTFULL_UI
        web_service->add_handler_file(HTML_STRIP, HTTP_ANY, RESP_HTML, HTML_STRIP
                ".gz", true);
        web_service->add_handler_file(JS_STRIP, HTTP_ANY, RESP_JS, JS_STRIP
                ".gz", true);
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
            resp += get_delay();
            resp += ", \"mode\" :";
            resp += get_transfer_mode();
            resp += ", \"type\" :";
            resp += get_type();
            return resp + "}";
        }, true);
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
        }, true);
        web_service->add_handler("/led-strip/set-speed", HTTP_POST, RESP_JSON, [this](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int32_t delay = parseJSON<int32_t>(json, "speed", -1);
            if (delay >= 0) {
                set_delay((uint16_t) ((255 - (uint8_t) (delay))));
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, true);
        web_service->add_handler("/led-strip/set-color", HTTP_POST, RESP_JSON, [this](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int rgb = parseJSON<int>(json, "rgb", -1),
                    hsb = parseJSON<int>(json, "hsb", -1);
            if (rgb >= 0) set_color((uint32_t) rgb);
            else if (hsb >= 0) set_color((uint32_t) hsb);
            else return JSON_RESP_NOK;
            return JSON_RESP_OK;
        }, true);
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
                                 }, true);
    }
};

#endif

#endif //ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_RESTFULL_H
