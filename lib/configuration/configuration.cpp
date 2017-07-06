#include "configuration.h"

template<typename T>
static T parseJSON(JsonObject &json, const char *arg, T default_value = 0) {
    if (json.containsKey(arg) && json[arg].is<T>()) {
        return json[arg].as<T>();
    }
    return default_value;
}

RestService *init_rest(RestService *web_service, REST_INIT scope) {
    if ((scope & HTML_ADMIN_FILES) == HTML_ADMIN_FILES) {
        web_service->add_handler_file(HTML_ADMINISTRATION, HTTP_ANY, RESP_HTML, HTML_ADMINISTRATION
                ".gz", true);
        web_service->add_handler_file(JS_ADMINISTRATION, HTTP_ANY, RESP_JS, JS_ADMINISTRATION
                ".gz", true);
    }
    if ((scope & HTML_STATUS_FILES) == HTML_STATUS_FILES) {
        web_service->add_handler_file(HTML_STATUS, HTTP_ANY, RESP_HTML, HTML_STATUS
                ".gz", true);
        web_service->add_handler_file(JS_STATUS, HTTP_ANY, RESP_JS, JS_STATUS
                ".gz", true);
    }
    if ((scope & HTML_COMMON_FILES) == HTML_COMMON_FILES) {
        web_service->add_handler_file("/", HTTP_ANY, RESP_HTML, HTML_INDEX
                ".gz", true);
        web_service->add_handler_file(JS_COMMON, HTTP_ANY, RESP_JS, JS_COMMON
                ".gz");
        web_service->add_handler_file(CSS_COMMON, HTTP_ANY, RESP_CSS, CSS_COMMON
                ".gz");
        web_service->add_handler_file(CSS_INDEX, HTTP_ANY, RESP_CSS, CSS_INDEX
                ".gz");
        web_service->add_handler_file(CSS_LOGIN, HTTP_ANY, RESP_CSS, CSS_LOGIN
                ".gz");
        web_service->add_handler_file("/get-config-global", HTTP_ANY, RESP_JSON, CONFIG_GLOBAL_JSON, true);
    }
    if ((scope & CALLBACKS_SYSTEM) == CALLBACKS_SYSTEM) {
        web_service->add_handler("/set-config-global", HTTP_POST, RESP_JSON, [](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const char *rest_acc = parseJSON<const char *>(json, "rest-acc"),
                    *rest_pass = parseJSON<const char *>(json, "rest-pass");
            if (rest_acc != NULL) {
                ConfigJSON::set<const char *>(CONFIG_GLOBAL_JSON, {"rest-acc"}, rest_acc);
            }
            if (rest_pass != NULL) {
                ConfigJSON::set<const char *>(CONFIG_GLOBAL_JSON, {"rest-pass"}, rest_pass);
            }
            return JSON_RESP_OK;
        }, true);
        web_service->add_handler("/get-system-info", HTTP_GET, RESP_JSON,
        [](String arg) -> String {
            String resp = "{\"chip-id\":";
            resp += ESP.getFlashChipId();
            resp += ",\"up-time\":";
            resp += (system_get_rtc_time() *
                     system_rtc_clock_cali_proc() *
                     1000000); // seconds
            resp += ",\"firmware\":\""
            FIRMWARE;
            resp += "\",\"model\":\" Wemos D1 mini";
            return resp + "\"}";
        },
        true);
        web_service->add_handler("/get-mem-info", HTTP_GET, RESP_JSON,
        [](String arg) -> String {
            String resp = "{\"sketch-mem-total\":";
            resp += ESP.getFlashChipRealSize();
            resp += ",\"sketch-mem-free\":";
            resp += ESP.getFreeSketchSpace();
            resp += ",\"heap-total\":64000"; // bytes
            resp += ",\"heap-free\":";
            resp += ESP.getFreeHeap();
            return resp + "}";
        },
        true);
        web_service->add_handler("/get-cpu-info", HTTP_GET, RESP_JSON,
        [](String arg) -> String {
            String resp = "{\"cpu-freq-max\":";
            resp += 346;
            resp += ",\"cpu-freq-cur\":";
            resp += ESP.getCpuFreqMHz();
            return resp + "}";
        },
        true);
        web_service->add_handler("/restart", HTTP_POST, RESP_JSON,
        [](String arg) -> String {
            ESP.restart();
            return JSON_RESP_OK;
        },
        true);
        web_service->add_handler("/reset-config", HTTP_POST, RESP_JSON,
        [](String arg) -> String {
            set_wifi_config_reset(true);
            return JSON_RESP_OK;
        },
        true);
    }
    if ((scope & CALLBACKS_WIFI) == CALLBACKS_WIFI) {
        web_service->add_handler("/get-config-wifi", HTTP_ANY, RESP_JSON,
        [](String arg) -> String {
            char *host_name = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"host-name"});
            String resp = "{\"mode\":";
            resp += WiFi.getMode();
            resp += ",\"mac\":";
            resp += "\"" + WiFi.macAddress() + "\"";
            resp += ",\"hostname\":\"";
            resp += host_name;
            checked_free(host_name);
            return resp + "\"}";
        }, true);
        web_service->add_handler("/set-config-wifi", HTTP_POST, RESP_JSON,
        [](String arg) -> String {
            StaticJsonBuffer<200> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const char *hostname =
                    parseJSON<const char *>(json, "hostname");
            if (hostname != NULL) {
                MDNS.setInstanceName(hostname);
                WiFi.hostname(hostname);
                ConfigJSON::set<const char *>(CONFIG_GLOBAL_JSON, {"host-name"}, hostname);
            }
            const int8_t mode = parseJSON<int8_t>(json, "mode", -1);
            if (mode != -1) {
                WiFi.mode((WiFiMode_t) mode);
            }
            if (json.containsKey("mac")) {
                uint8_t mac[6];
                JsonArray &array = json["mac"].as<JsonArray>();
                if (array.size() == 6) {
                    for (uint8_t i = 0; i < array.size(); i++) {
                        mac[i] = array[i].as<uint8_t>();
                    }
                    WiFi.macAddress(mac);
                }
            }
            return JSON_RESP_OK;
        },
        true);
        web_service->add_handler("/get-config-wifi-ap", HTTP_ANY, RESP_JSON,
        [](String arg) -> String {
            String resp = "{\"ssid\":\"";
            softap_config ap_config;
            wifi_softap_get_config(&ap_config);
            resp += reinterpret_cast<char *>(ap_config.ssid);
            resp += "\",\"channel\":";
            resp += ap_config.channel;
            resp += ",\"hidden\":";
            resp += ap_config.ssid_hidden;
            resp += ",\"ip\":";
            resp += "\"" + WiFi.softAPIP().toString() + "\"";
            return resp + "}";
        }, true);
        web_service->add_handler(
                "/set-config-wifi-ap", HTTP_POST, RESP_JSON,
        [](String arg) -> String {
            StaticJsonBuffer<200> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const char *ssid = parseJSON<const char *>(json, "ssid"),
                    *passphrase = parseJSON<const char *>(json, "pass", NULL);
            const uint8_t channel = parseJSON<uint8_t>(json, "channel", 1),
                    ssid_hidden = parseJSON<uint8_t>(json, "hidden", 0);
            if (ssid != NULL) {
                WiFi.softAP(ssid, passphrase, channel, ssid_hidden);
            } else
                return JSON_RESP_NOK;
            return JSON_RESP_OK;
        },
        true);
        web_service->add_handler(
                "/get-config-wifi-sta", HTTP_ANY, RESP_JSON, [](String arg) -> String {
            String resp = "{\"ssid\":\"";
            station_config sta_config;
            wifi_station_get_ap_info(&sta_config);
            resp += reinterpret_cast<char *>(sta_config.ssid);
            resp += "\",\"status\":";
            resp += WiFi.isConnected();
            resp += ",\"ip\":";
            resp += "\"" + WiFi.localIP().toString() + "\"";
            return resp + "}";
        }, true);
        web_service->add_handler(
                "/set-config-wifi-sta", HTTP_POST, RESP_JSON,
        [](String arg) -> String {
            StaticJsonBuffer<200> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const char *ssid = parseJSON<const char *>(json, "ssid"),
                    *passphrase = parseJSON<const char *>(json, "pass", NULL);
            if (ssid != NULL) {
                WiFi.softAPdisconnect(true);
                WiFi.begin(ssid, passphrase);
            } else
                return JSON_RESP_NOK;
            return JSON_RESP_OK;
        },
        true);
        web_service->add_handler("/get-networks", HTTP_GET, RESP_JSON,
        [](String arg) -> String {
            String resp = "{ \"networks\":[";
            int8_t networks = WiFi.scanNetworks();
            for (uint8_t i = 0; i < networks; i++) {
                resp += i == 0 ? "\"" + WiFi.SSID(i) + "\""
                               : ",\"" + WiFi.SSID(i) + "\"";
            }
            return resp + "]}";
        },
        true);
    }
    if ((scope & HTML_DIGITAL_IO) == HTML_DIGITAL_IO) {
        web_service->add_handler_file(HTML_RELAY, HTTP_ANY, RESP_HTML, HTML_RELAY
                ".gz", true);
        web_service->add_handler_file(JS_RELAY, HTTP_ANY, RESP_JS, JS_RELAY
                ".gz", true);
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
            std::list < Device * > devices;
            Devices::get_devices(DIGITAL_IO, &devices);
            String resp = "{ \"devices\" : [";
            for (std::list<Device *>::iterator i = devices.begin(); i != devices.end(); i++) {
                if (i != devices.begin()) {
                    resp += ",";
                }
                resp += "{ \"id\":";
                resp += (*i)->get_id();
                resp += ", \"state\":";
                resp += ((DigitalIO * )(*i))->get_state();
                resp += "}";
            }
            return resp + "]}";
        }, true);
    }
    if ((scope & LOGGING) == LOGGING) {
        web_service->add_handler_file(HTML_LOG, HTTP_ANY, RESP_HTML, HTML_LOG
                ".gz", true);
        web_service->add_handler_file(JS_LOG, HTTP_ANY, RESP_JS, JS_LOG
                ".gz", true);
        WiFi.onStationModeGotIP([](WiFiEventStationModeGotIP e) {
            Log::println("Got IP:%s", e.ip.toString().c_str());
        });
        WiFi.onStationModeDisconnected(
        [](WiFiEventStationModeDisconnected e) {
            Log::println("WiFi lost connection [%s]", e.ssid.c_str());
        });
        WiFi.onStationModeConnected([](WiFiEventStationModeConnected e) {
            Log::println("WiFi accepted [%s]", e.ssid.c_str());
        });
        WiFi.onSoftAPModeStationConnected([](WiFiEventSoftAPModeStationConnected e) {
            Log::println("Client connected");
        });
        WiFi.onSoftAPModeStationDisconnected([](WiFiEventSoftAPModeStationDisconnected e) {
            Log::println("Client disconnected");
        });
    }
    return web_service;
}

RestService *init_rest(RestService *web_service, LedStripService *led_service, REST_INIT scope) {
    if ((scope & HTML_LED_STRIP_FILES) == HTML_LED_STRIP_FILES) {
        web_service->add_handler_file(HTML_STRIP, HTTP_ANY, RESP_HTML, HTML_STRIP
        ".gz", true);
        web_service->add_handler_file(JS_STRIP, HTTP_ANY, RESP_JS, JS_STRIP
        ".gz", true);
        web_service->add_handler("/led-strip/get-config", HTTP_GET, RESP_JSON, [led_service](String arg) -> String {
            String resp = "{ \"animation-type\" :";
            resp += led_service->get_mode();
            resp += ", \"length\" :";
            resp += led_service->get_len();
            resp += ", \"color\" :";
            resp += led_service->get_color();
            resp += ", \"brightness\" :";
            resp += led_service->get_brightness();
            resp += ", \"speed\" :";
            resp += led_service->get_delay();
            return resp + "}";
        }, true);
        web_service->add_handler("/led-strip/set-config", HTTP_POST, RESP_JSON, [](String arg) -> String {
            // TODO:
            return JSON_RESP_OK;
        }, true);
        web_service->add_handler("/led-strip/set-mode", HTTP_POST, RESP_JSON, [led_service](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int16_t mode = parseJSON<int8_t>(json, "mode", -1);
            if (mode >= 0) {
                led_service->set_mode((LED_STRIP_MODE) mode);
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, true);
        web_service->add_handler("/led-strip/set-speed", HTTP_POST, RESP_JSON, [led_service](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int32_t delay = parseJSON<int32_t>(json, "speed", -1);
            if (delay >= 0) {
                led_service->set_delay((uint16_t) delay);
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, true);
        web_service->add_handler("/led-strip/set-color", HTTP_POST, RESP_JSON, [led_service](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const uint32_t color = parseJSON<uint32_t>(json, "color", 0);
            led_service->set_color((uint8_t)((0x00FF0000 & color) >> 16), (uint8_t)((0x0000FF00 & color) >> 8),
                                   (uint8_t)(0x000000FF & color));
            return JSON_RESP_OK;
        }, true);
        web_service->add_handler("/led-strip/set-brightness", HTTP_POST, RESP_JSON, [led_service](String arg) -> String {
            StaticJsonBuffer<100> jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(arg);
            if (!json.success())
                return JSON_RESP_NOK;
            const int8_t brightness = parseJSON<int8_t>(json, "brightness", -1);
            if (brightness >= 0) {
                led_service->set_brightness((uint8_t) brightness);
                return JSON_RESP_OK;
            }
            return JSON_RESP_NOK;
        }, true);
    }
    return init_rest(web_service, scope);
}