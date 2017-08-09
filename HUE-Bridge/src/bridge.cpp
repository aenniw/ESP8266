#include <Arduino.h>
#include <vector>

#include <commons.h>
#include <service_log.h>
#include <service_rest.h>
#include <service_hue_bridge.h>

const char *ssid = "****";
const char *password = "****";

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
    Log::init();

    auto *ledstrip = new LedStripService(GRB, UART800, 1);
    ledstrip->set_brightness(0);
    ledstrip->cycle_routine();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Log::println("\nConnecting to wifi ");
    SPIFFS.begin();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Log::print(".");
    }
    Log::println(".");

    auto *restService = new RestService(nullptr, nullptr, 80);
    auto *hueBridge = new HueBridge(restService);
    hueBridge->add_light(ledstrip);

    services.push_back(Log::getInstance());
    services.push_back((Service *) restService);
    services.push_back((Service *) ledstrip);
    services.push_back((Service *) hueBridge);
    Log::println("Staring.");
}

void loop() {
    for (auto &service : services) {
        service->cycle_routine();
        yield(); // WATCHDOG/WIFI feed
    }
}