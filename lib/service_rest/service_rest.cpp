#include "service_rest.h"

template<typename T>
static T parseJSON(JsonObject &json, const char *arg, T default_value = 0) {
    if (json.containsKey(arg) && json[arg].is<T>()) {
        return json[arg].as<T>();
    }
    return default_value;
}

RestService::RestService(const char *user, const char *pass,
                         const uint16_t port) {
    if (user != NULL && pass != NULL && strlen(user) > 0 && strlen(pass) > 0) {
        acc = (char *) malloc(sizeof(char) * (1 + strlen(user)));
        strcpy(acc, user);
        passwd = (char *) malloc(sizeof(char) * (1 + strlen(pass)));
        strcpy(passwd, pass);
        login_id = generate_login_id();
    }
    web_server = new ESP8266WebServer(port);
    const char *headerkeys[] = {"Cookie"};
    web_server->collectHeaders(headerkeys, 1);
    web_server->onNotFound([this]() { on_not_found(); });
    web_server->on("/login", [=]() {
        if (!valid_credentials()) {
            on_invalid_credentials();
        } else if (HTTP_GET == web_server->method()) {
            String header("HTTP/1.1 301 OK\r\nSet-Cookie: LOGINID=");
            header += login_id;
            header += "\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
            web_server->sendContent(header);
        } else
            on_not_found();
    });
    web_server->on("/logout", [=]() {
        if (!valid_credentials())
            return on_invalid_credentials();
        if (HTTP_GET == web_server->method()) {
            web_server->sendContent("HTTP/1.1 301 OK\r\nSet-Cookie: "
                                            "LOGINID=0\r\nLocation: /\r\nCache-Control: "
                                            "no-cache\r\n\r\n");
        } else
            on_not_found();
    });
    web_server->begin();
}

void RestService::on_not_found() {
#ifdef __DEBUG__
    String message = "Not Found\n\nURI: ";
    message += web_server->uri();
    message += "\nMethod: ";
    message += (web_server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nHeaders: ";
    message += web_server->headers();
    message += "\n";
    for (uint8_t i = 0; i < web_server->headers(); i++) {
        message +=
                " " + web_server->headerName(i) + ": " + web_server->header(i) + "\n";
    }
    message += "\nArguments: " + web_server->args();
    message += "\n";
    for (uint8_t i = 0; i < web_server->args(); i++) {
        message += " " + web_server->argName(i) + ": " + web_server->arg(i) + "\n";
    }
    web_server->send(404, RESP_TEXT, message);
#else
    web_server->send(404, RESP_TEXT, "Not found");
#endif
}

uint32_t RestService::generate_login_id() {
    uint32_t id = ESP.getChipId() + ESP.getVcc();
    for (int i = strlen(acc); i >= 0; i--) {
        id += acc[i];
    }
    for (int i = strlen(passwd); i >= 0; i--) {
        id += passwd[i];
    }
    return id;
}

void RestService::on_invalid_credentials() {
    File file = SPIFFS.open(HTML_LOGIN".gz", "r");
    if (file) {
        web_server->streamFile(file, RESP_HTML);
        file.close();
    } else {
        web_server->requestAuthentication();
    }
}

bool RestService::valid_credentials() {
    if (acc == NULL || passwd == NULL) {
        return true;
    }
    if (web_server->hasHeader("Cookie")) {
        String valid_cookie("LOGINID=");
        valid_cookie += login_id;
        if (web_server->header("Cookie").indexOf(valid_cookie) != -1) {
            return true;
        }
    }
    return web_server->authenticate(acc, passwd);
}

void RestService::add_handler(const char *uri, HTTPMethod method,
                              const char *resp_type,
                              RestServiceFunction handler,
                              const bool authentication) {
    web_server->on(uri, [=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            web_server->send(200, resp_type, handler(web_server->arg("plain")));
        } else
            on_not_found();
    });
}

void RestService::add_handler_file(const char *uri, HTTPMethod method,
                                   const char *resp_type, const char *file_name,
                                   const bool authentication) {
    web_server->on(uri, [=]() {
        Log::println("Recieved on %s %s", authentication ? "Auth" : "NAuth", uri);
        if (authentication && !valid_credentials())
            return on_invalid_credentials();
        if (method == HTTP_ANY || method == web_server->method()) {
            File file = SPIFFS.open(file_name, "r");
            if (file) {
                web_server->streamFile(file, resp_type);
                file.close();
            } else {
                Log::println("File %s cannot be opened.", file_name);
                on_not_found();
            }
        } else
            on_not_found();
    });
}

void RestService::cycle_routine() { web_server->handleClient(); }

RestService *RestService::initialize(RestService *web_service,
                                     REST_INIT scope) {
    switch (scope) {
        case ALL:
            initialize(web_service, HTML);
            initialize(web_service, CALLBACKS_SYSTEM);
            initialize(web_service, CALLBACKS_WIFI);
            initialize(web_service, LOGGING);
            initialize(web_service, RELAYS);
            break;
        case HTML:
            // HTML
            web_service->add_handler_file("/", HTTP_ANY, RESP_HTML, HTML_INDEX ".gz",
                                          true);
            web_service->add_handler_file(HTML_ADMINISTRATION, HTTP_ANY, RESP_HTML,
                                          HTML_ADMINISTRATION ".gz", true);
            web_service->add_handler_file(HTML_STATUS, HTTP_ANY, RESP_HTML,
                                          HTML_STATUS ".gz", true);
            web_service->add_handler_file(HTML_WIFI, HTTP_ANY, RESP_HTML,
                                          HTML_WIFI ".gz", true);
            web_service->add_handler_file(HTML_LOG, HTTP_ANY, RESP_HTML, HTML_LOG ".gz",
                                          true);
            // JAVASCRIPT
            web_service->add_handler_file(JS_LOG, HTTP_ANY, RESP_JS, JS_LOG ".gz",
                                          true);
            web_service->add_handler_file(JS_ADMINISTRATION, HTTP_ANY, RESP_JS, JS_ADMINISTRATION".gz",
                                          true);
            web_service->add_handler_file(JS_STATUS, HTTP_ANY, RESP_JS, JS_STATUS".gz",
                                          true);
            web_service->add_handler_file(JS_COMMON, HTTP_ANY, RESP_JS,
                                          JS_COMMON ".gz");
            // CSS
            web_service->add_handler_file(CSS_COMMON, HTTP_ANY, RESP_CSS,
                                          CSS_COMMON ".gz");
            web_service->add_handler_file(CSS_INDEX, HTTP_ANY, RESP_CSS,
                                          CSS_INDEX ".gz");
            web_service->add_handler_file(CSS_LOGIN, HTTP_ANY, RESP_CSS,
                                          CSS_LOGIN ".gz");
            // JSON
            web_service->add_handler_file("/get-config-global", HTTP_ANY, RESP_JSON, CONFIG_GLOBAL_JSON, true);
            break;
        case CALLBACKS_SYSTEM:
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
            break;
        case CALLBACKS_WIFI:
            web_service->add_handler("/get-config-wifi", HTTP_ANY, RESP_JSON,
                                     [](String arg) -> String {
                                         String resp = "{\"mode\":";
                                         resp += WiFi.getMode();
                                         resp += ",\"mac\":";
                                         resp += "\"" + WiFi.macAddress() + "\"";
                                         resp += ",\"hostname\":\"";
                                         resp += WiFi.hostname();
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
                                             WiFi.hostname(hostname);
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
            break;
        case RELAYS:
            web_service->add_handler_file(HTML_RELAY, HTTP_ANY, RESP_HTML, HTML_RELAY".gz", true);
            web_service->add_handler_file(JS_RELAY, HTTP_ANY, RESP_JS, JS_RELAY ".gz", true);

            web_service->add_handler("/set-relay-state", HTTP_POST, RESP_JSON, [](String arg) -> String {
                StaticJsonBuffer<100> jsonBuffer;
                JsonObject &json = jsonBuffer.parseObject(arg);
                if (!json.success())
                    return JSON_RESP_NOK;
                const uint8_t pin = parseJSON<uint8_t>(json, "pin");
                const bool state = parseJSON<bool>(json, "state", false);
                Relay *d = (Relay *) Devices::get(pin);
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
            web_service->add_handler("/devices-get-relays", HTTP_GET, RESP_JSON, [](String arg) -> String {
                std::list<Device *> devices;
                Devices::get_devices(RELAY, &devices);
                String resp = "{ \"devices\" : [";
                for (std::list<Device *>::iterator i = devices.begin(); i != devices.end(); i++) {
                    if (i != devices.begin()) {
                        resp += ",";
                    }
                    resp += "{ \"id\":";
                    resp += (*i)->get_id();
                    resp += ", \"state\":";
                    resp += ((Relay *) (*i))->get_state();
                    resp += "}";
                }
                return resp + "]}";
            }, true);
            break;
        case LOGGING:
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
            break;
    }
    return web_service;
}
