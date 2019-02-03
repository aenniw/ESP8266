#ifndef ESP8266_PROJECTS_ROOT_SERVICE_REST_ROBUST_H
#define ESP8266_PROJECTS_ROOT_SERVICE_REST_ROBUST_H

#include <file_system.h>
#include <commons_json.h>
#include <service_rest.h>

#if defined(RESTFULL_UI) || defined(RESTFULL_CALLS)

#define HTML_INDEX              "/index.html"
#define HTML_ADMINISTRATION     "/html/administration.html"
#define HTML_STATUS             "/html/status.html"
#define HTML_WIFI               "/html/wifi.html"
#define HTML_LOG                "/html/log.html"
#define CSS_COMMON              "/css/common_style.css"
#define CSS_LOGIN               "/css/login_style.css"
#define JS_COMMON               "/js/common.js"
#define JS_LOG                  "/js/log.js"
#define JS_ADMINISTRATION       "/js/administration.js"
#define JS_STATUS               "/js/status.js"

typedef enum {
    ALL = 0xFFFFFFFF,
    CALLBACKS_WIFI = 0x1,
#ifdef RESTFULL_UI
    HTML_ALL_FILES = 0x2,
#endif
    CALLBACKS_SYSTEM = 0x4
} REST_INIT;

class RestServiceRobust : public RestService {
public:
    RestServiceRobust(const char *usr, const char *pass, const uint16_t p, const REST_INIT scope) :
            RestService(usr, pass, p) {
#ifdef RESTFULL_UI
        if ((scope & HTML_ALL_FILES) == HTML_ALL_FILES) {
            add_handler_file(HTML_ADMINISTRATION, HTTP_ANY, RESP_HTML, HTML_ADMINISTRATION
                    ".gz", true, true);
            add_handler_file(HTML_STATUS, HTTP_ANY, RESP_HTML, HTML_STATUS
                    ".gz", true, true);
            add_handler_file(HTML_LOG, HTTP_ANY, RESP_HTML, HTML_LOG
                    ".gz", true, true);
            add_handler_file("/", HTTP_ANY, RESP_HTML, HTML_INDEX
                    ".gz", true, true);
            add_handler_file(JS_ADMINISTRATION, HTTP_ANY, RESP_JS, JS_ADMINISTRATION
                    ".gz", true, true);
            add_handler_file(JS_STATUS, HTTP_ANY, RESP_JS, JS_STATUS
                    ".gz", true, true);
            add_handler_file(JS_COMMON, HTTP_ANY, RESP_JS, JS_COMMON
                    ".gz", false, true);
            add_handler_file(JS_LOG, HTTP_ANY, RESP_JS, JS_LOG
                    ".gz", true, true);
            add_handler_file(CSS_COMMON, HTTP_ANY, RESP_CSS, CSS_COMMON
                    ".gz", false, true);
            add_handler_file(CSS_LOGIN, HTTP_ANY, RESP_CSS, CSS_LOGIN
                    ".gz", false, true);
            add_handler_file("/get-config-global", HTTP_ANY, RESP_JSON, CONFIG_GLOBAL_JSON, true);
        }
#endif
        if ((scope & CALLBACKS_SYSTEM) == CALLBACKS_SYSTEM) {
            add_handler("/set-config-global", HTTP_POST, RESP_JSON, [](String arg) -> String {
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
            add_handler("/get-system-info", HTTP_GET, RESP_JSON,
                        [](String arg) -> String {
                            String resp = "{\"chip-id\":";
                            resp += ESP.getFlashChipId();
                            resp += ",\"up-time\":";
                            resp += millis();
                            resp += ",\"firmware\":\"" FIRMWARE;
                            resp += "\",\"model\":\" Wemos D1 mini";
                            return resp + "\"}";
                        },
                        true);
            add_handler("/get-mem-info", HTTP_GET, RESP_JSON,
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
            add_handler("/get-cpu-info", HTTP_GET, RESP_JSON,
                        [](String arg) -> String {
                            String resp = "{\"cpu-freq-max\":";
                            resp += 346;
                            resp += ",\"cpu-freq-cur\":";
                            resp += ESP.getCpuFreqMHz();
                            return resp + "}";
                        },
                        true);
            add_handler("/restart", HTTP_POST, RESP_JSON,
                        [](String arg) -> String {
                            ESP.restart();
                            return JSON_RESP_OK;
                        },
                        true);
            add_handler("/reset-config", HTTP_POST, RESP_JSON,
                        [](String arg) -> String {
                            set_wifi_config_reset(true);
                            return JSON_RESP_OK;
                        },
                        true);
        }
        if ((scope & CALLBACKS_WIFI) == CALLBACKS_WIFI) {
            add_handler("/get-config-wifi", HTTP_ANY, RESP_JSON,
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
            add_handler("/set-config-wifi", HTTP_POST, RESP_JSON,
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
            add_handler("/get-config-wifi-ap", HTTP_ANY, RESP_JSON,
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
            add_handler(
                    "/set-config-wifi-ap", HTTP_POST, RESP_JSON,
                    [](String arg) -> String {
                        StaticJsonBuffer<200> jsonBuffer;
                        JsonObject &json = jsonBuffer.parseObject(arg);
                        if (!json.success())
                            return JSON_RESP_NOK;
                        const char *ssid = parseJSON<const char *>(json, "ssid"),
                                *passphrase = parseJSON<const char *>(json, "pass");
                        const uint8_t channel = parseJSON<uint8_t>(json, "channel", 1),
                                ssid_hidden = parseJSON<uint8_t>(json, "hidden", 0);
                        if (ssid != NULL && passphrase != NULL) {
                            WiFi.softAP(ssid, passphrase, channel, ssid_hidden);
                        } else
                            return JSON_RESP_NOK;
                        return JSON_RESP_OK;
                    },
                    true);
            add_handler(
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
            add_handler(
                    "/set-config-wifi-sta", HTTP_POST, RESP_JSON,
                    [](String arg) -> String {
                        StaticJsonBuffer<200> jsonBuffer;
                        JsonObject &json = jsonBuffer.parseObject(arg);
                        if (!json.success())
                            return JSON_RESP_NOK;
                        const char *ssid = parseJSON<const char *>(json, "ssid"),
                                *passphrase = parseJSON<const char *>(json, "pass");
                        if (ssid != NULL && passphrase != NULL) {
                            WiFi.begin(ssid, passphrase);
                        } else
                            return JSON_RESP_NOK;
                        return JSON_RESP_OK;
                    },
                    true);
            add_handler("/get-networks", HTTP_GET, RESP_JSON,
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
    }
};

#endif

#endif //ESP8266_PROJECTS_ROOT_SERVICE_REST_ROBUST_H
