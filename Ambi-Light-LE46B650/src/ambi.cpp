#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NeoPixelBus.h>

#define WIFI_SSID "****"
#define WIFI_PASS "****"
#define HORIZONTAL_LEDS 60
#define VERTICAL_LEDS 34

WiFiUDP *udp_socket;
NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> *strip;

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
        uint8_t buffer[6 * (HORIZONTAL_LEDS + VERTICAL_LEDS)];
        udp_socket->readBytes(buffer, 6 * (HORIZONTAL_LEDS + VERTICAL_LEDS));
        for (uint16_t p = 0; p < 2 * (HORIZONTAL_LEDS + VERTICAL_LEDS); p++) {
            strip->SetPixelColor(p, RgbColor(buffer[0 + 3 * p], buffer[1 + 3 * p], buffer[2 + 3 * p]));
        }
        strip->Show();
    }
    yield(); // WATCHDOG/WIFI feed
}
