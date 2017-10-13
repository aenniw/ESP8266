#include <Arduino.h>
#include <vector>

#define BRIDGE

#include <commons.h>
#include <service_log.h>
#include <FS.h>
#include <service_ota.h>
#include <service_shard.h>
#include <service_hue_bridge.h>
#ifndef BRIDGE
#include <service_led_strip_restfull.h>
#endif
const char *ssid = "**";
const char *password = "**";

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
    Log::init();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Log::println("\nConnecting to wifi.");
    SPIFFS.begin();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Log::print(".");
    }
    Log::println(".\nConnected. IP %s", WiFi.localIP().toString().c_str());

    auto *restService = new RestService(nullptr, nullptr, 80);
    services.push_back(Log::getInstance());
    services.push_back(OtaService::get_instance("admin"));
    services.push_back((Service *) restService);


#ifndef BRIDGE
    MDNS.addService("hue", "tcp", 80);
    services.push_back((Service *) new RestFullLedStripService(GRB, UART800, 1, restService));
#else
    services.push_back((Service *) new HueBridge(restService));
#endif
    Log::println("Staring.");
}


void loop() {
    for (auto &service : services) {
        service->cycle_routine();
        yield(); // WATCHDOG/WIFI feed
    }
}