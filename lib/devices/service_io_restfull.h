#ifndef ESP8266_PROJECTS_ROOT_SERVICE_IO_RESTFULL_H
#define ESP8266_PROJECTS_ROOT_SERVICE_IO_RESTFULL_H

#include <devices.h>
#include <service_rest.h>

#if defined(RESTFULL_UI) || defined(RESTFULL_CALLS)

#define HTML_RELAY "/html/svc/d-io.html"
#define JS_RELAY "/js/svc/d-io.js"

class RestFullIO : public Devices {
public:
    static void register_callbacks(RestService *web_service) {
#ifdef RESTFULL_UI
        web_service->add_handler_file(HTML_RELAY, HTTP_ANY, RESP_HTML, HTML_RELAY
                ".gz", true);
        web_service->add_handler_file(JS_RELAY, HTTP_ANY, RESP_JS, JS_RELAY
                ".gz", true);
#endif
        web_service->add_handler("/set-d-io-state", HTTP_POST, RESP_JSON, [](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const uint8_t pin = parseJSON<uint8_t>(json, "pin");
            const bool state = parseJSON<bool>(json, "state", false);
            DigitalIO *d = (DigitalIO *) Devices::get(pin);
            if (d == NULL) {
                return JSON_RESP_NOK;
            }
            d->set_state(state);
            return JSON_RESP_OK;
        }, true);
        web_service->add_handler("/devices-add", HTTP_POST, RESP_JSON, [](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const uint8_t pin = parseJSON<uint8_t>(json, "pin");
            const DEVICE_TYPE type = (const DEVICE_TYPE) parseJSON<int8_t>(json, "type", -1);
            if (Devices::put(pin, type) != NULL) {
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, true);
        web_service->add_handler("/devices-remove", HTTP_POST, RESP_JSON, [](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const uint8_t pin = parseJSON<uint8_t>(json, "pin");
            if (Devices::remove(pin)) {
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, true);
        web_service->add_handler("/devices-get-available-pins", HTTP_GET, RESP_JSON, [](String arg) -> String {
            String resp = "{ \"pins\":[";
            uint32_t len = 0;
            const uint8_t *pins = Devices::get_pins(&len);
            for (uint32_t i = 0; i < len; i++) {
                resp += pins[i];
                if (i + 1 < len) {
                    resp += ",";
                }
            }
            return resp + "]}";
        }, true);
        web_service->add_handler("/devices-get-d-io", HTTP_GET, RESP_JSON, [](String arg) -> String {
            std::list<Device *> devices;
            Devices::get_devices(DIGITAL_IO, &devices);
            String resp = "{ \"devices\" : [";
            for (std::list<Device *>::iterator i = devices.begin(); i != devices.end(); i++) {
                if (i != devices.begin()) {
                    resp += ",";
                }
                resp += "{ \"id\":";
                resp += (*i)->get_id();
                resp += ", \"state\":";
                resp += ((DigitalIO *) (*i))->get_state();
                resp += "}";
            }
            return resp + "]}";
        }, true);
    }
};

#endif

#endif //ESP8266_PROJECTS_ROOT_SERVICE_IO_RESTFULL_H
