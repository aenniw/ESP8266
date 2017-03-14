#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <NeoPixelBus.h>
#include <WiFiUdp.h>

// TODO: remove, these needs to be changeable during runtime/reboot per user
#define WIFI_SSID "****"
#define WIFI_PASS "****"
#define HORIZONTAL_LEDS 60
#define VERTICAL_LEDS 34
// TODO: this may be shifted to LE46B650 logic if possible
#define SMOOTH_FADING

static WiFiUDP *udp_socket;
static int missed_ticks = 0;
static NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> *strip;

void ICACHE_FLASH_ATTR setup() {
    Serial.begin(115200);
    strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod>(2 * (HORIZONTAL_LEDS + VERTICAL_LEDS), 0);
    strip->Begin();
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    // TODO: if not connected in few second go to AP mode, to be able to reflash or update settings of module.
    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }
    udp_socket = new WiFiUDP();
    udp_socket->begin(65000);
    ArduinoOTA.setPort(8266);
    ArduinoOTA.begin();
    delay(100);
}

void loop() {
    if (udp_socket->parsePacket()) {
        if (missed_ticks != 0) missed_ticks = 0;
        uint8_t buffer[6 * (HORIZONTAL_LEDS + VERTICAL_LEDS)];
        udp_socket->readBytes(buffer, 6 * (HORIZONTAL_LEDS + VERTICAL_LEDS));
        for (uint16_t p = 0; p < 2 * (HORIZONTAL_LEDS + VERTICAL_LEDS); p++) {
#ifndef SMOOTH_FADING
            strip->SetPixelColor(p, RgbColor(buffer[0 + 3 * p], buffer[1 + 3 * p], buffer[2 + 3 * p]));
#else
            RgbColor current_color = strip->GetPixelColor(p);
            strip->SetPixelColor(p, RgbColor((uint8_t) (0.5 * (buffer[0 + 3 * p] + current_color.R)),
                                             (uint8_t) (0.5 * (buffer[1 + 3 * p] + current_color.G)),
                                             (uint8_t) (0.5 * (buffer[2 + 3 * p] + current_color.B))));
#endif
        }
        strip->Show();
    } else if (missed_ticks >= 500000) {
        for (uint16_t p = 0; p < 2 * (HORIZONTAL_LEDS + VERTICAL_LEDS); p++) {
            strip->SetPixelColor(p, RgbColor(0, 0, 0));
        }
        strip->Show();
        missed_ticks = -1;
    } else if (missed_ticks >= 0) {
        missed_ticks++;
    } else {
        ArduinoOTA.handle();
        delay(500);
    }
    yield(); // WATCHDOG/WIFI feed
}
