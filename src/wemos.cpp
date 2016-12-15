#include <Arduino.h>
#include <FS.h>
#include <vector>
#include <ArduinoJson.h>

#include <API.h>
#include <web_service.h>
#include <led_strip.h>
#include <ir.h>
#include <ntp_provider.h>
#include <ota.h>
#include <file_system.h>

#define CONFIG_GLOBAL "/json/config-global.json"
#define CONFIG_STRIP "/json/config-strip.json"
#define CONFIG_WIFI "/json/config-wifi.json"

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
#define JS_STRIP "/js/strip_requests.js"

#ifdef __ESP_01__
#define D3 0
#define D4 2
#endif

std::vector<ESP_Service *> services;

WebService *setup_web_service(RGBStrip *strip_service, const char *acc, const char *pass) {
    WebService *web_service = new WebService(acc, pass);
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
    //Config files
    web_service->add_handler_file("/get-config-global", HTTP_ANY, RESP_JSON, CONFIG_GLOBAL);
    web_service->add_handler_file("/get-config-strip", HTTP_ANY, RESP_JSON, CONFIG_STRIP);
    web_service->add_handler_file("/get-config-wifi", HTTP_ANY, RESP_JSON, CONFIG_WIFI);
    // REST strip delay
    web_service->add_handler_auth("/set-strip-delay", HTTP_POST, RESP_JSON, [=](String arg) -> String {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success() || !json.containsKey("delay"))
            return JSON_RESP_NOK;
        strip_service->set_delay(json["delay"].as<uint32_t>());
        return JSON_RESP_OK;
    });
    // REST strip mode
    web_service->add_handler_auth("/set-strip-mode", HTTP_POST, RESP_JSON, [=](String arg) -> String {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success() || !json.containsKey("mode"))
            return JSON_RESP_NOK;
        strip_service->set_mode((STRIP_MODES) json["mode"].as<uint8_t>());
        return JSON_RESP_OK;
    });
    // REST strip brightness
    web_service->add_handler_auth("/set-strip-brightness", HTTP_POST, RESP_JSON, [=](String arg) -> String {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success() || !json.containsKey("brightness"))
            return JSON_RESP_NOK;
        strip_service->set_brightness(json["brightness"].as<uint8_t>());
        return JSON_RESP_OK;
    });
    // REST strip color
    web_service->add_handler_auth("/set-strip-color", HTTP_POST, RESP_JSON, [=](String arg) -> String {
        StaticJsonBuffer<150> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success() || !json.containsKey("color") ||
            !json["color"]["r"].is<uint8_t>() || !json["color"]["g"].is<uint8_t>() || !json["color"]["b"].is<uint8_t>())
            return JSON_RESP_NOK;
        strip_service->set_color(json["color"]["r"].as<uint8_t>(), json["color"]["g"].as<uint8_t>(),
                                 json["color"]["b"].as<uint8_t>());
        return JSON_RESP_OK;
    });
    //reset
    web_service->add_handler_auth("/restart", HTTP_POST, RESP_JSON, [=](String arg) -> String {
        ESP.restart();
        return JSON_RESP_OK;
    });
    return web_service;
}

IR *setup_ir_service(RGBStrip *strip_service) {
    IR *ir_service = new IR(D3, 100);
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

Time *setup_ntp_service() {
    Time *ntp_service = new Time(7777);
    ntp_service->set_time_zone(UTC_01_00);
    ntp_service->update_time_ntp();
    return ntp_service;
}

void setup() {
#ifdef __DEBUG__
    Serial.begin(115200);
    Serial.println();
#endif
    if (ESP.getResetInfoPtr()->reason == REASON_SOFT_WDT_RST || !SPIFFS.begin()) {
#ifdef __DEBUG__
        Serial.println(ESP.getResetInfo());
        Serial.println(ESP.getResetReason());
#endif
        services.push_back(new FAILSAFE_Service());
        WiFi.mode(WIFI_AP);
        WiFi.softAP("AP-FAILSAFE");
        return;
    }

    wifi_status_led_install(LED_BUILTIN, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    const WiFiMode wifi_mode = (WiFiMode) get_config<uint8_t>(CONFIG_WIFI, {"mode"});
    WiFi.mode(wifi_mode);
    if (wifi_mode == WIFI_AP || wifi_mode == WIFI_AP_STA) {
        const char
                *ap_ssid = get_config<const char *>(CONFIG_WIFI, {"ap", "ssid"}),
                *ap_pass = get_config<const char *>(CONFIG_WIFI, {"ap", "pass"});
        const bool ap_hidden = get_config<bool>(CONFIG_WIFI, {"ap", "hidden"});
        const uint8_t ap_channel = get_config<uint8_t>(CONFIG_WIFI, {"ap", "channel"});
        WiFi.softAP(ap_ssid, ap_pass, ap_channel, ap_hidden);
        //TODO free heap or reuse in instances
        free((void *) ap_ssid);
        free((void *) ap_pass);
    }
    if (wifi_mode == WIFI_STA || wifi_mode == WIFI_AP_STA) {
        const char
                *client_ssid = get_config<const char *>(CONFIG_WIFI, {"client", "ssid"}),
                *client_pass = get_config<const char *>(CONFIG_WIFI, {"client", "pass"});
        WiFi.begin(client_ssid, client_pass);
        //TODO free heap or reuse in instances
        free((void *) client_ssid);
        free((void *) client_pass);
    }
    const uint16_t led_count = get_config<uint16_t>(CONFIG_STRIP, {"led-count"});
    const STRIP_MODES strip_mode = (STRIP_MODES) get_config<uint8_t>(CONFIG_STRIP, {"strip-mode"});
    const char *admin_acc = get_config<const char *>(CONFIG_GLOBAL, {"acc"}),
            *admin_pass = get_config<const char *>(CONFIG_GLOBAL, {"pass"});

    services.push_back(new FAILSAFE_Service(admin_pass));
    RGBStrip *strip_service = new LedStrip<NeoEsp8266Uart800KbpsMethod>(led_count, 10);
    strip_service->set_mode(strip_mode);
    services.push_back((ESP_Service *&&) strip_service);
    services.push_back(setup_web_service(strip_service, admin_acc, admin_pass));
    services.push_back(setup_ir_service(strip_service));
    services.push_back(setup_ntp_service());

    //TODO free heap or reuse in instances
    free((void *) admin_acc);
    free((void *) admin_pass);
    if (WiFi.getMode() == WIFI_OFF) WiFi.forceSleepBegin();
}


void loop() {
    if (WiFi.getMode() == WIFI_OFF) WiFi.forceSleepWake();
    //TODO migrate strip animation out of loop
    for (std::vector<ESP_Service *>::iterator i = services.begin(); i != services.end(); i++)
        (*i)->cycle_routine();
    yield(); // WATCHDOG/WIFI feed
}
