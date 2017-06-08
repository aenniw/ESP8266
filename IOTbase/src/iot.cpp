#include <Arduino.h>
#include <vector>

#include <commons.h>
#include <file_system.h>
#include <service_rest.h>
#include <service_ota.h>

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
    Log::init();
    if (!SPIFFS.begin()) {
        Log::println("SPIFFS failed to initialize flash corrupted?");
    }
    {   // Services setup
        char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
                *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"});
        services.push_back(Log::getInstance());
        services.push_back(OtaService::get_instance(admin_pass));
        services.push_back(
                RestService::initialize(new RestService(admin_acc, admin_pass, 80), ALL));
        Log::println("Credentials: [%s:%s]", admin_acc, admin_pass);
        checked_free(admin_acc);
        checked_free(admin_pass);
    }
    wifi_config_reset();
    Devices::parse_devices(CONFIG_IO_JSON, DIGITAL_IO);
    delay(500);
}

void loop() {
    for (std::vector<Service *>::iterator i = services.begin();
         i != services.end(); i++)
        (*i)->cycle_routine();
    yield(); // WATCHDOG/WIFI feed
}
