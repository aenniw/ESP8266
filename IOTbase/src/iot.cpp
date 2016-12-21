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
#include <service_ntp_time.h>

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

#ifdef ARDUINO_ESP8266_ESP01
#define D3 0
#define D4 2
#define MODEL "ESP01"
#else
#define MODEL "Wemos D1 mini"
#endif

std::vector<ESP_Service *> services;

void ICACHE_FLASH_ATTR setup_front_end(RestService *web_service) {
    //HTML
    web_service->add_handler_file_auth("/", HTTP_ANY, RESP_HTML, HTML_INDEX);
    web_service->add_handler_file_auth(HTML_ADMINISTRATION, HTTP_ANY, RESP_HTML, HTML_ADMINISTRATION);
    web_service->add_handler_file_auth(HTML_STATUS, HTTP_ANY, RESP_HTML, HTML_STATUS);
    web_service->add_handler_file_auth(HTML_STRIP, HTTP_ANY, RESP_HTML, HTML_STRIP);
    web_service->add_handler_file_auth(HTML_WIFI, HTTP_ANY, RESP_HTML, HTML_WIFI);
    web_service->add_handler_file_auth(HTML_IR, HTTP_ANY, RESP_HTML, HTML_IR);
    //JAVASCRIPT
    web_service->add_handler_file_auth(JS_STRIP, HTTP_ANY, RESP_JS, JS_STRIP);
    web_service->add_handler_file_auth(JS_TIME, HTTP_ANY, RESP_JS, JS_TIME);
    web_service->add_handler_file(JS_COMMON, HTTP_ANY, RESP_JS, JS_COMMON);
    //CSS
    web_service->add_handler_file(CSS_COMMON, HTTP_ANY, RESP_CSS, CSS_COMMON);
    web_service->add_handler_file(CSS_INDEX, HTTP_ANY, RESP_CSS, CSS_INDEX);
    web_service->add_handler_file(CSS_LOGIN, HTTP_ANY, RESP_CSS, CSS_LOGIN);
}

void ICACHE_FLASH_ATTR setup_back_end_status(RestService *web_service) {
    web_service->add_handler_file_auth("/get-config-global", HTTP_ANY, RESP_JSON, CONFIG_GLOBAL);
    web_service->add_handler_auth("/set-config-global", HTTP_POST, RESP_JSON, [](String arg) -> String {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success())
            return JSON_RESP_NOK;
        if (json.containsKey("acc"))
            set_config<const char *>(CONFIG_GLOBAL, {"acc"}, json["acc"].asString());
        if (json.containsKey("pass"))
            set_config<const char *>(CONFIG_GLOBAL, {"pass"}, json["pass"].asString());
        return JSON_RESP_OK;
    });
    web_service->add_handler_auth("/get-system-info", HTTP_GET, RESP_JSON, [](String arg) -> String {
        String resp = "{\"chip-id\":";
        resp += ESP.getFlashChipId();
        resp += ",\"up-time\":";
        resp += (system_get_rtc_time() * system_rtc_clock_cali_proc() * 1000000); //seconds
        resp += ",\"firmware\":\"" FIRMWARE;
        resp += "\",\"model\":\"" MODEL;
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
        return JSON_RESP_OK;
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

void ICACHE_FLASH_ATTR setup_back_end_strip(RestService *web_service, RGBStrip *strip_service) {
    web_service->add_handler_file_auth("/get-config-strip", HTTP_ANY, RESP_JSON, CONFIG_STRIP);
    web_service->add_handler_auth("/set-config-strip", HTTP_POST, RESP_JSON, [=](String arg) -> String {
        StaticJsonBuffer<300> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success())
            return JSON_RESP_NOK;
        bool persist = json.containsKey("persist") ? json["persist"].as<bool>() : false;
        if (json.containsKey("delay") && json["delay"].is<uint32_t>()) {
            Serial.println("set-strip-delay");
            strip_service->set_delay(json["delay"].as<uint32_t>());
            if (persist) set_config<uint32_t>(CONFIG_STRIP, {"delay"}, json["delay"].as<uint32_t>());
        }
        if (json.containsKey("mode") && json["mode"].is<int8_t>()) {
            Serial.println("set-strip-mode");
            strip_service->set_mode((STRIP_MODES) json["mode"].as<int8_t>());
            if (persist) set_config<int8_t>(CONFIG_STRIP, {"mode"}, json["mode"].as<int8_t>());
        }
        if (json.containsKey("brightness") && json["brightness"].is<uint8_t>()) {
            Serial.println("set-strip-brightness");
            strip_service->set_brightness(json["brightness"].as<uint8_t>());
            if (persist) set_config<uint8_t>(CONFIG_STRIP, {"brightness"}, json["brightness"].as<uint8_t>());
        }
        // FIXME: migrate ARGB format
        if (json.containsKey("color") && json["color"].is<uint32_t>()) {
            Serial.println("set-strip-color");
            strip_service->set_color((uint8_t) ((json["color"].as<uint32_t>() & 0x00FF0000) >> 16),
                                     (uint8_t) ((json["color"].as<uint32_t>() & 0x0000FF00) >> 8),
                                     (uint8_t) ((json["color"].as<uint32_t>() & 0x000000FF)));
            if (persist) set_config<uint32_t>(CONFIG_STRIP, {"color"}, json["color"].as<uint32_t>());
        }
        return JSON_RESP_OK;
    });
}

void ICACHE_FLASH_ATTR setup_back_end(RestService *web_service, RGBStrip *strip_service) {
    // TODO TEST SYSTEM STATUS
    setup_back_end_status(web_service);
    // TODO TEST STRIP CALLBACKS
    setup_back_end_strip(web_service, strip_service);
    //TODO TEST WIFI CALLBACKS
    setup_back_end_wifi(web_service);
}

RestService *ICACHE_FLASH_ATTR setup_web_service(RGBStrip *strip_service, const char *acc, const char *pass) {
    RestService *web_service = new RestService(acc, pass, 80);
    setup_front_end(web_service);
    setup_back_end(web_service, strip_service);
    return web_service;
}

IRService *ICACHE_FLASH_ATTR setup_ir_service(RGBStrip *strip_service) {
    IRService *ir_service = new IRService(D3);
    ir_service->add_handler(0xFFF807, [=]() { strip_service->set_mode(OFF); });
    ir_service->add_handler(0xFFB04F, [=]() {
        strip_service->set_color(strip_service->get_color().R,
                                 strip_service->get_color().G,
                                 strip_service->get_color().B);
    });
    // COLORS
    ir_service->add_handler(0xFFA857, [=]() { strip_service->set_color(255, 255, 255); });
    ir_service->add_handler(0xFF9867, [=]() { strip_service->set_color(255, 0, 0); });
    ir_service->add_handler(0xFFD827, [=]() { strip_service->set_color(0, 255, 0); });
    ir_service->add_handler(0xFF8877, [=]() { strip_service->set_color(0, 0, 255); });

    ir_service->add_handler(0xFFE817, [=]() { strip_service->set_color(102, 0, 0); });
    ir_service->add_handler(0xFF48B7, [=]() { strip_service->set_color(51, 102, 0); });
    ir_service->add_handler(0xFF6897, [=]() { strip_service->set_color(0, 51, 153); });

    ir_service->add_handler(0xFF02FD, [=]() { strip_service->set_color(255, 51, 0); });
    ir_service->add_handler(0xFF32CD, [=]() { strip_service->set_color(0, 102, 204); });
    ir_service->add_handler(0xFF20DF, [=]() { strip_service->set_color(51, 0, 51); });

    ir_service->add_handler(0xFF50AF, [=]() { strip_service->set_color(255, 153, 0); });
    ir_service->add_handler(0xFF7887, [=]() { strip_service->set_color(0, 51, 51); });
    ir_service->add_handler(0xFF708F, [=]() { strip_service->set_color(102, 0, 51); });

    ir_service->add_handler(0xFF38C7, [=]() { strip_service->set_color(255, 120, 0); });
    ir_service->add_handler(0xFF28D7, [=]() { strip_service->set_color(0, 26, 26); });
    ir_service->add_handler(0xFFF00F, [=]() { strip_service->set_color(153, 0, 51); });
    // BRIGHTNESS
    ir_service->add_handler(0xFF906F, [=]() {
        if (strip_service->get_mode() == RAINBOW || strip_service->get_mode() == RAINBOW_CYCLE)
            strip_service->set_delay(strip_service->get_delay() >= 995 ? 1000 : strip_service->get_delay() + 5);
        else
            strip_service->set_brightness(
                    (uint8_t) (strip_service->get_brightness() >= 95 ? 100 : strip_service->get_brightness() + 5));
    });
    ir_service->add_handler(0xFFB847, [=]() {
        if (strip_service->get_mode() == RAINBOW || strip_service->get_mode() == RAINBOW_CYCLE)
            strip_service->set_delay(strip_service->get_delay() <= 5 ? 5 : strip_service->get_delay() - 5);
        else
            strip_service->set_brightness(
                    (uint8_t) (strip_service->get_brightness() <= 5 ? 5 : strip_service->get_brightness() - 5));
    });
    //MODES
    ir_service->add_handler(0xFFB24D, [=]() { strip_service->set_mode(RAINBOW); });
    ir_service->add_handler(0xFF00FF, [=]() { strip_service->set_mode(RAINBOW_CYCLE); });
    return ir_service;
}

void ICACHE_FLASH_ATTR setup() {
    Log::init();
    if (!SPIFFS.begin()) Log::println("SPIFFS failed to initialize flash corrupted?");
    if (ESP.getResetInfoPtr()->reason == REASON_SOFT_WDT_RST) {
        Log::println(ESP.getResetInfo());
        Log::println(ESP.getResetReason());
    }
    WiFi.persistent(true);
    wifi_status_led_install(LED_BUILTIN, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    const uint16_t led_count = get_config<uint16_t>(CONFIG_STRIP, {"led-count"}),
            strip_delay = get_config<uint16_t>(CONFIG_STRIP, {"delay"});
    const STRIP_MODES strip_mode = (STRIP_MODES) get_config<uint8_t>(CONFIG_STRIP, {"mode"});
    const char *admin_acc = get_config<const char *>(CONFIG_GLOBAL, {"acc"}),
            *admin_pass = get_config<const char *>(CONFIG_GLOBAL, {"pass"});

    services.push_back(OtaService::get_instance(admin_pass));
    RGBStrip *strip_service = new LedStrip<NeoEsp8266Uart800KbpsMethod>(led_count, strip_delay);
    strip_service->set_mode(strip_mode);
    services.push_back((ESP_Service *&&) strip_service);
    services.push_back(setup_web_service(strip_service, admin_acc, admin_pass));
    services.push_back(setup_ir_service(strip_service));
    services.push_back(NtpTimeService::get_instance(7777));

    free((void *) admin_acc);
    free((void *) admin_pass);
}

void loop() {
    if (WiFi.getSleepMode() != WIFI_NONE_SLEEP) WiFi.forceSleepWake();
    for (std::vector<ESP_Service *>::iterator i = services.begin(); i != services.end(); i++)
        (*i)->cycle_routine();
    yield(); // WATCHDOG/WIFI feed
    if (WiFi.getMode() == WIFI_OFF) WiFi.forceSleepBegin();
}
