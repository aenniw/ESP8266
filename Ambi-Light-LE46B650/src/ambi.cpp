#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NeoPixelBus.h>

#define WIFI_SSID "****"
#define WIFI_PASS "****"
#define HORIZONTAL_LEDS 60
#define VERTICAL_LEDS 34

static WiFiUDP *udp_socket;
static int missed_ticks = 0;
static NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> *strip;

void ICACHE_FLASH_ATTR setup() {
    Serial.begin(115200);
    strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod>(2 * (HORIZONTAL_LEDS + VERTICAL_LEDS), 0);
    strip->Begin();
    WiFi.mode(WIFI_AP);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }
    udp_socket = new WiFiUDP();
    udp_socket->begin(65000);
    delay(100);
}

void loop() {
    if (udp_socket->parsePacket()) {
        if (missed_ticks != 0) missed_ticks = 0;
        uint8_t buffer[6 * (HORIZONTAL_LEDS + VERTICAL_LEDS)];
        udp_socket->readBytes(buffer, 6 * (HORIZONTAL_LEDS + VERTICAL_LEDS));
        for (uint16_t p = 0; p < 2 * (HORIZONTAL_LEDS + VERTICAL_LEDS); p++) {
            strip->SetPixelColor(p, RgbColor(buffer[0 + 3 * p], buffer[1 + 3 * p], buffer[2 + 3 * p]));
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
        delay(500);
    }
    yield(); // WATCHDOG/WIFI feed
}
