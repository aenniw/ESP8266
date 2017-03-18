#include <Arduino.h>
#include <FS.h>
#include <vector>
#include <ArduinoJson.h>

#include <esp_api.h>
#include <service_rest.h>
#include <led_strip.h>
#include <service_ir.h>
#include <file_system.h>
#include <service_ota.h>

#define CONFIG_GLOBAL "/json/config-global.json"
#define CONFIG_STRIP "/json/config-strip.json"

#define HTML_INDEX "/index.html"
#define HTML_ADMINISTRATION "/html/administration.html"
#define HTML_STATUS "/html/status.html"
#define HTML_STRIP "/html/strip.html"
#define HTML_WIFI "/html/wifi.html"
#define HTML_IR "/html/ir.html"

#define CSS_COMMON "/css/common_style.css"
#define CSS_INDEX "/css/index_style.css"
#define CSS_LOGIN "/css/login_style.css"

#define JS_COMMON "/js/common.js"
#define JS_TIME "/js/time_requests.js"
#define JS_STRIP "/js/strip.js"

std::vector<ESP_Service *> services;

void ICACHE_FLASH_ATTR setup_front_end(RestService *web_service) {
    //HTML
    web_service->add_handler_file("/", HTTP_ANY, RESP_HTML, HTML_INDEX".gz", true);
    web_service->add_handler_file(HTML_ADMINISTRATION, HTTP_ANY, RESP_HTML, HTML_ADMINISTRATION".gz", true);
    web_service->add_handler_file(HTML_STATUS, HTTP_ANY, RESP_HTML, HTML_STATUS".gz", true);
    web_service->add_handler_file(HTML_STRIP, HTTP_ANY, RESP_HTML, HTML_STRIP".gz", true);
    web_service->add_handler_file(HTML_WIFI, HTTP_ANY, RESP_HTML, HTML_WIFI".gz", true);
    web_service->add_handler_file(HTML_IR, HTTP_ANY, RESP_HTML, HTML_IR".gz", true);
    //JAVASCRIPT
    web_service->add_handler_file(JS_STRIP, HTTP_ANY, RESP_JS, JS_STRIP".gz", true);
    web_service->add_handler_file(JS_TIME, HTTP_ANY, RESP_JS, JS_TIME".gz", true);
    web_service->add_handler_file(JS_COMMON, HTTP_ANY, RESP_JS, JS_COMMON".gz");
    //CSS
    web_service->add_handler_file(CSS_COMMON, HTTP_ANY, RESP_CSS, CSS_COMMON".gz");
    web_service->add_handler_file(CSS_INDEX, HTTP_ANY, RESP_CSS, CSS_INDEX".gz");
    web_service->add_handler_file(CSS_LOGIN, HTTP_ANY, RESP_CSS, CSS_LOGIN".gz");
}

void ICACHE_FLASH_ATTR setup_back_end_status(RestService *web_service) {
    web_service->add_handler_file_auth("/get-config-global", HTTP_ANY, RESP_JSON, CONFIG_GLOBAL);
    web_service->add_handler_auth("/set-config-global", HTTP_POST, RESP_JSON, [](String arg) -> String {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success())
            return JSON_RESP_NOK;
        if (json.containsKey("acc"))
            set_config<const char *>(CONFIG_GLOBAL, {"acc"}, json["acc"].as<char *>());
        if (json.containsKey("pass"))
            set_config<const char *>(CONFIG_GLOBAL, {"pass"}, json["pass"].as<char *>());
        return JSON_RESP_OK;
    });
    web_service->add_handler_auth("/get-system-info", HTTP_GET, RESP_JSON, [](String arg) -> String {
        String resp = "{\"chip-id\":";
        resp += ESP.getFlashChipId();
        resp += ",\"up-time\":";
        resp += (system_get_rtc_time() * system_rtc_clock_cali_proc() * 1000000); //seconds
        resp += ",\"firmware\":\"" FIRMWARE;
        resp += "\",\"model\":\" Wemos D1 mini";
        return resp + "\"}";
    });
    web_service->add_handler_auth("/get-mem-info", HTTP_GET, RESP_JSON, [](String arg) -> String {
        String resp = "{\"sketch-mem-total\":";
        resp += ESP.getFlashChipRealSize();
        resp += ",\"sketch-mem-free\":";
        resp += ESP.getFreeSketchSpace();
        resp += ",\"heap-total\":64000"; //bytes
        resp += ",\"heap-free\":";
        resp += ESP.getFreeHeap();
        return resp + "}";
    });
    web_service->add_handler_auth("/get-cpu-info", HTTP_GET, RESP_JSON, [](String arg) -> String {
        String resp = "{\"cpu-freq-max\":";
        resp += 346;
        resp += ",\"cpu-freq-cur\":";
        resp += ESP.getCpuFreqMHz();
        return resp + "}";
    });
    web_service->add_handler_auth("/restart", HTTP_POST, RESP_JSON, [](String arg) -> String {
        ESP.restart();
        return JSON_RESP_OK;
    });
}

// FIXME use permanent memory of ESP WiFi
void ICACHE_FLASH_ATTR setup_back_end_wifi(RestService *web_service) {
    web_service->add_handler_auth("/get-config-wifi", HTTP_ANY, RESP_JSON, [](String arg) -> String {
        String resp = "{\"wifi-mode\":";
        resp += WiFi.getMode();
        resp += ",\"mac\":";
        resp += "\"" + WiFi.macAddress() + "\"";
        resp += ",\"hostname\":";
        resp += "\"" + WiFi.hostname() + "\"";
        // STA
        station_config sta_config;
        wifi_station_get_ap_info(&sta_config);
        resp += ",\"sta-ssid\":\"";
        resp += reinterpret_cast<char *>(sta_config.ssid);
        resp += "\",\"sta-status\":";
        resp += WiFi.isConnected();
        resp += ",\"sta-ip\":";
        resp += "\"" + WiFi.localIP().toString() + "\"";
        // AP
        softap_config ap_config;
        wifi_softap_get_config(&ap_config);
        resp += ",\"ap-ssid\":\"";
        resp += reinterpret_cast<char *>(ap_config.ssid);
        resp += "\",\"ap-channel\":";
        resp += ap_config.channel;
        resp += ",\"ap-hidden\":";
        resp += ap_config.ssid_hidden;
        resp += ",\"ap-auth-mode\":";
        resp += ap_config.authmode;
        resp += ",\"ap-ip\":";
        resp += "\"" + WiFi.softAPIP().toString() + "\"";
        return resp + "}";
    });
    web_service->add_handler_auth("/set-config-wifi", HTTP_POST, RESP_JSON, [](String arg) -> String {
        StaticJsonBuffer<300> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success())
            return JSON_RESP_NOK;

        return JSON_RESP_OK;
    });

    web_service->add_handler_auth("/get-networks", HTTP_GET, RESP_JSON, [](String arg) -> String {
        String resp = "{ \"networks\":[";
        int8_t networks = WiFi.scanNetworks();
        for (uint8_t i = 0; i < networks; i++) {
            resp += i == 0 ?
                    "\"" + WiFi.SSID(i) + "\"" :
                    ",\"" + WiFi.SSID(i) + "\"";
        }
        return resp + "]}";
    });
}


void ICACHE_FLASH_ATTR setup_back_end(RestService *web_service) {
    // TODO TEST SYSTEM STATUS
    setup_back_end_status(web_service);
    //TODO TEST WIFI CALLBACKS
    setup_back_end_wifi(web_service);
}

RestService *ICACHE_FLASH_ATTR setup_web_service(const char *acc, const char *pass) {
    RestService *web_service = new RestService(acc, pass, 80);
    setup_front_end(web_service);
    setup_back_end(web_service);
    return web_service;
}


void ICACHE_FLASH_ATTR setup() {
    Log::init();
    if (!SPIFFS.begin()) Log::println("SPIFFS failed to initialize flash corrupted?");
    if (ESP.getResetInfoPtr()->reason == REASON_SOFT_WDT_RST) {
        Log::println(ESP.getResetInfo());
        Log::println(ESP.getResetReason());
    }
    wifi_status_led_install(LED_BUILTIN, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    WiFi.mode(WIFI_STA);
    const char *admin_acc = "admin", *admin_pass = "admin";

    services.push_back(OtaService::get_instance(admin_pass));
    services.push_back(setup_web_service(admin_acc, admin_pass));
}

void loop() {
    for (std::vector<ESP_Service *>::iterator i = services.begin(); i != services.end(); i++)
        (*i)->cycle_routine();
    yield(); // WATCHDOG/WIFI feed
}
