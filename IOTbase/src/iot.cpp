#include <Arduino.h>
#include <vector>

#include <commons.h>
#include <file_system.h>
#include <service_rest.h>
#include <service_ota.h>
#include <service_led_strip.h>
#include <configuration.h>

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
    Log::init();
    if (!SPIFFS.begin()) {
        Log::println("SPIFFS failed to initialize flash corrupted?");
    }
    {   // Services setup
        char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
                *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"}),
                *host_name = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"host-name"});
        services.push_back(Log::getInstance());
        services.push_back(OtaService::get_instance(admin_pass));
        LedStripService *led_strip = new LedStripService(1);
        services.push_back(init_rest(new RestService(admin_acc, admin_pass, 80), led_strip, ALL));
        services.push_back((Service *) led_strip);
        Log::println("Credentials: [%s:%s]", admin_acc, admin_pass);
        if (MDNS.begin(host_name)) {
            Log::println("Hostname: [%s]", host_name);
        }
        checked_free(admin_acc);
        checked_free(admin_pass);
        checked_free(host_name);
    }
    wifi_config_reset();
    Devices::parse_devices(CONFIG_IO_JSON, DIGITAL_IO);
    delay(500);
}

void loop() {
    for (std::vector<Service *>::iterator i = services.begin();
         i != services.end(); i++) {
        (*i)->cycle_routine();
        yield(); // WATCHDOG/WIFI feed
    }
}
