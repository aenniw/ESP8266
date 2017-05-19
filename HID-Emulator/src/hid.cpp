#include <Arduino.h>
#include <vector>

#include <commons.h>
#include <file_system.h>
#include <service_rest.h>
#include <service_ota.h>

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
    Serial.begin(115200);
#ifdef __DEBUG__
    Log::init();
#endif
    if (!SPIFFS.begin()) {
#ifdef __DEBUG__
        Log::println("SPIFFS failed to initialize flash corrupted?");
#endif
    }
    {   // Services setup
        char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
                *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"});
#ifdef __DEBUG__
        services.push_back(Log::getInstance());
#endif
        services.push_back(OtaService::get_instance(admin_pass));
        services.push_back(
                RestService::initialize(new RestService(admin_acc, admin_pass, 80),
                                        CALLBACKS_SYSTEM | CALLBACKS_WIFI | HTML));
#ifdef __DEBUG__
        Log::println("Credentials: [%s:%s]", admin_acc, admin_pass);
#endif
        checked_free(admin_acc);
        checked_free(admin_pass);
    }
    wifi_config_reset();
    delay(500);
}

void loop() {
    for (std::vector<Service *>::iterator i = services.begin();
         i != services.end(); i++)
        (*i)->cycle_routine();
    yield(); // WATCHDOG/WIFI feed
}
