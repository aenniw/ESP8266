#include <Arduino.h>
#include <FS.h>

#include <aJSON.h>
#include <file_system.h>
#include <ir.h>
#include <led_strip.h>
#include <ntp_provider.h>
#include <web_service.h>

#ifdef __ESP_01__
static const uint8_t D0 = 3;
static const uint8_t D1 = 4;
#endif

char *SSID_local = NULL, *PASSWD_local = NULL, *SSID_remote = NULL,
        *PASSWD_remote = NULL;
LedStrip<NeoEsp8266Uart800KbpsMethod> *strip_service;
WebService *web_service;
Time *time_service;
IR *ir_service;

void setup_strip_REST() {
    // REST strip delay
    web_service->add_handler_auth(
            "/set-strip-delay", HTTP_POST, RESP_TEXT, [](String arg) -> String {
                aJsonObject *data = aJson.parse((char *) arg.c_str());
                for (aJsonObject *item = data->child; item != NULL; item = item->next) {
                    if (strcmp(item->name, "delay") == 0) {
                        strip_service->set_delay((uint32_t) item->valueint);
                        free(data);
                        return "{ \"result\" : true }";
                    }
                }
                free(data);
                return "{ \"result\" : false }";
            });

    // REST strip mode
    web_service->add_handler_auth(
            "/set-strip-mode", HTTP_POST, RESP_TEXT, [](String arg) -> String {
                aJsonObject *data = aJson.parse((char *) arg.c_str());
                for (aJsonObject *item = data->child; item != NULL; item = item->next) {
                    if (strcmp(item->name, "mode") == 0) {
                        strip_service->set_mode((STRIP_MODES) item->valueint);
                        free(data);
                        return "{ \"result\" : true }";
                    }
                }
                free(data);
                return "{ \"result\" : false }";
            });
    // REST strip brightness
    web_service->add_handler_auth(
            "/set-strip-brightness", HTTP_POST, RESP_TEXT, [](String arg) -> String {
                aJsonObject *data = aJson.parse((char *) arg.c_str());
                for (aJsonObject *item = data->child; item != NULL; item = item->next) {
                    if (strcmp(item->name, "brightness") == 0) {
                        strip_service->set_brightness((STRIP_MODES) item->valueint);
                        free(data);
                        return "{ \"result\" : true }";
                    }
                }
                free(data);
                return "{ \"result\" : false }";
            });
    // REST strip color
    web_service->add_handler_auth(
            "/set-strip-color", HTTP_POST, RESP_TEXT, [](String arg) -> String {
                aJsonObject *data = aJson.parse((char *) arg.c_str());
                uint8_t r = 0, g = 0, b = 0;
                bool valid = false;
                for (aJsonObject *item = data->child; item != NULL; item = item->next) {
                    if (strcmp(item->name, "color") == 0) {
                        item = item->child;
                        valid = true;
                    }
                    if (strcmp(item->name, "r") == 0)
                        r = (uint8_t) item->valueint;
                    if (strcmp(item->name, "g") == 0)
                        g = (uint8_t) item->valueint;
                    if (strcmp(item->name, "b") == 0)
                        b = (uint8_t) item->valueint;
                }
                if (valid) {
                    strip_service->set_color(r, g, b);
                    free(data);
                    return "{ \"result\" : true }";
                }
                free(data);
                return "{ \"result\" : false }";
            });
}

void setup_IR_codes() {
    ir_service->add_handler(0xFFF807, []() { strip_service->set_mode(OFF); });
    ir_service->add_handler(0xFFB04F, []() {
        strip_service->set_color(strip_service->get_color().R,
                                 strip_service->get_color().G,
                                 strip_service->get_color().B);
    });
    // COLORS
    ir_service->add_handler(0xFFA857, []() { strip_service->set_color(255, 255, 255); });
    ir_service->add_handler(0xFF9867, []() { strip_service->set_color(255, 0, 0); });
    ir_service->add_handler(0xFFD827, []() { strip_service->set_color(0, 255, 0); });
    ir_service->add_handler(0xFF8877, []() { strip_service->set_color(0, 0, 255); });

    ir_service->add_handler(0xFFE817, []() { strip_service->set_color(102, 0, 0); });
    ir_service->add_handler(0xFF48B7, []() { strip_service->set_color(51, 102, 0); });
    ir_service->add_handler(0xFF6897, []() { strip_service->set_color(0, 51, 153); });

    ir_service->add_handler(0xFF02FD, []() { strip_service->set_color(255, 51, 0); });
    ir_service->add_handler(0xFF32CD, []() { strip_service->set_color(0, 102, 204); });
    ir_service->add_handler(0xFF20DF, []() { strip_service->set_color(51, 0, 51); });

    ir_service->add_handler(0xFF50AF, []() { strip_service->set_color(255, 153, 0); });
    ir_service->add_handler(0xFF7887, []() { strip_service->set_color(0, 51, 51); });
    ir_service->add_handler(0xFF708F, []() { strip_service->set_color(102, 0, 51); });

    ir_service->add_handler(0xFF38C7, []() { strip_service->set_color(255, 120, 0); });
    ir_service->add_handler(0xFF28D7, []() { strip_service->set_color(0, 26, 26); });
    ir_service->add_handler(0xFFF00F, []() { strip_service->set_color(153, 0, 51); });
    // BRIGHTNESS
    ir_service->add_handler(0xFF906F, []() {
        if (strip_service->get_mode() == RAINBOW || strip_service->get_mode() == RAINBOW_CYCLE)
            strip_service->set_delay(strip_service->get_delay() >= 995 ? 1000 : strip_service->get_delay() + 5);
        else
            strip_service->set_brightness(
                    (uint8_t) (strip_service->get_brightness() >= 95 ? 100 : strip_service->get_brightness() + 5));
    });
    ir_service->add_handler(0xFFB847, []() {
        if (strip_service->get_mode() == RAINBOW || strip_service->get_mode() == RAINBOW_CYCLE)
            strip_service->set_delay(strip_service->get_delay() <= 5 ? 5 : strip_service->get_delay() - 5);
        else
            strip_service->set_brightness(
                    (uint8_t) (strip_service->get_brightness() <= 5 ? 5 : strip_service->get_brightness() - 5));
    });
    //MODES
    ir_service->add_handler(0xFFB24D, []() { strip_service->set_mode(RAINBOW); });
    ir_service->add_handler(0xFF00FF, []() { strip_service->set_mode(RAINBOW_CYCLE); });
}

void setup() {
#ifdef __DEBUG__
    Serial.begin(115200);
#endif
    strip_service = new LedStrip<NeoEsp8266Uart800KbpsMethod>(49, 100);
    web_service = new WebService("admin", "admin");
    time_service = new Time(7777);
    ir_service = new IR(D1, 100);

    // TODO rework
    WiFi.softAP(SSID_local, PASSWD_local);
    WiFi.begin(SSID_remote, PASSWD_remote);
    if (!SPIFFS.begin() || !load_config())
        Serial.println("Failed to load config");

    // setup_OTA(8266, SSID_local, PASSWD_local);
    // listen_for_OTA();
    web_service->add_handler("/", HTTP_ANY, RESP_HTML, [](String arg) -> String {
        return get_file_content("/index.html");
    });
    setup_strip_REST();
    setup_IR_codes();

    time_service->set_time_zone(UTC_01_00);
    strip_service->set_mode(RAINBOW);
}

void loop() {
    web_service->cycle_routine();
    time_service->cycle_routine();
    strip_service->cycle_routine();
}
