#include <Arduino.h>
#include <ESP8266mDNS.h>

#include <commons.h>
#include <file_system.h>
#include <service_ota.h>
#include <service_rest_robust.h>
#include <service_io_restfull.h>
#include <service_led_strip_persist.h>

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
    Log::init();
    if (!SPIFFS.begin()) {
        Log::println("SPIFFS failed to initialize flash corrupted?");
    }

    config_reset_check(); // Double pressing reset button causes wifi config reset.

    char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
            *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"});

    auto rest = new RestServiceRobust(admin_acc, admin_pass, 80, ALL);

    services.push_back(Log::getInstance());
    services.push_back(OtaService::get_instance(admin_pass));
    services.push_back((Service *) rest);
    services.push_back(new PersistentLedStripService(rest));
    Log::println("Credentials: [%s:%s]", admin_acc, admin_pass);

    checked_free(admin_acc);
    checked_free(admin_pass);

    RestFullIO::register_callbacks(rest);
    RestFullIO::parse_devices(CONFIG_IO_JSON, DIGITAL_IO);

    WiFi.begin();
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);

    Log::println("Starting.");
    for (auto &service : services)
        service->begin();
    Log::println("Started.");
}

void mdns_cycle() {
    static bool started = false;
    if (started) {
        MDNS.update();
    } else {
        char *host_name = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"host-name"});
        if (WiFi.hostname(host_name) && MDNS.close() && MDNS.begin(host_name)) {
            started = true;
            Log::println("Hostname: [%s]", host_name);
        }
        checked_free(host_name);
    }
}

void loop() {
    for (auto &service : services) {
        service->cycle_routine();
        yield(); // WATCHDOG/WIFI feed
    }
    mdns_cycle();
}