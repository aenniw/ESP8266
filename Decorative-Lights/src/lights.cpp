#include <Arduino.h>
#include <ESP8266mDNS.h>

#include <commons.h>
#include <NIButtons.h>
#include <file_system.h>
#include <service_ota.h>
#include <service_rest_robust.h>
#include <service_led_strip_persist.h>

NIButton cycle_button(D8);

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
    Log::init();
    if (!SPIFFS.begin()) {
        Log::println("SPIFFS failed to initialize flash corrupted?");
        return;
    }
    config_reset_check(); // Double pressing reset button causes wifi config reset.

    char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
            *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"}),
            *host_name = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"host-name"});

    auto rest = new RestServiceRobust(admin_acc, admin_pass, 80, ALL);
    auto strip = new PersistentLedStripService(rest, false);

    services.push_back(Log::getInstance());
    services.push_back(OtaService::get_instance(admin_pass));
    services.push_back((Service *) rest);
    services.push_back((Service *) strip);
    services.push_back(&cycle_button);

    Log::println("Credentials: [%s:%s]", admin_acc, admin_pass);
    if (MDNS.begin(host_name)) {
        MDNS.setInstanceName(host_name);
        Log::println("Hostname: [%s]", host_name);
    }

    checked_free(admin_acc);
    checked_free(admin_pass);
    checked_free(host_name);

    cycle_button.on_click([=]() { strip->set_mode(LED_STRIP_ANIM_MODE((strip->get_mode() + 1) % (OFF + 1))); });
    cycle_button.on_hold([=]() {
        strip->set_brightness((uint8_t) (strip->get_brightness() + 5 + (5 - strip->get_brightness() % 5)));
    });

    Log::println("Starting.");
    for (auto &service : services)
        service->begin();
    Log::println("Started.");
}

void loop() {
    for (auto &service : services) {
        service->cycle_routine();
        yield(); // WATCHDOG/WIFI feed
    }
    MDNS.update();
}