#include <Arduino.h>
#include <vector>

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

    // Double pressing reset button causes wifi config reset.
    config_reset_check();

    Log::println("Starting up.");
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);

    { // Services setup
        char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
                *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"}),
                *host_name = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"host-name"});
        services.push_back(Log::getInstance());
        services.push_back(OtaService::get_instance(admin_pass));
        services.push_back(new RestServiceRobust(admin_acc, admin_pass, 80, ALL));
        RestFullIO::register_callbacks((RestService *) services.back());
        services.push_back(new PersistentLedStripService((RestService *) services.back()));
        Log::println("Credentials: [%s:%s]", admin_acc, admin_pass);
        if (MDNS.begin(host_name)) {
            Log::println("Hostname: [%s]", host_name);
        }
        checked_free(admin_acc);
        checked_free(admin_pass);
        checked_free(host_name);
    }
    RestFullIO::parse_devices(CONFIG_IO_JSON, DIGITAL_IO);
    delay(500);
}

void loop() {
    for (auto &service : services) {
        service->cycle_routine();
        yield(); // WATCHDOG/WIFI feed
    }
}