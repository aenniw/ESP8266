#include <Arduino.h>
#include <vector>

#include <commons.h>
#include <file_system.h>
#include <service_ota.h>
#include <service_rest_robust.h>
#include <service_led_strip_persist.h>

#define CYCLE_BUTTON D8

std::vector<Service *> services;
LedStripService *strip = nullptr;

void ledStripCycleModesCallback();
void ICACHE_FLASH_ATTR setup() {
    Log::init();
    if (!SPIFFS.begin()) {
        Log::println("SPIFFS failed to initialize flash corrupted?");
        return;
    }

    // Double pressing reset button causes wifi config reset.
    pinMode(LED_BUILTIN, OUTPUT);
    if (!get_wifi_config_reset()) {
        wifi_config_reset();
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
        set_wifi_config_reset(true);
        Log::println("Waiting for config reset event.");
        delay(3000);
        set_wifi_config_reset(false);
        digitalWrite(LED_BUILTIN, LOW);
    }

    Log::println("Starting up.");
    {   // Services setup
        char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
                *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"}),
                *host_name = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"host-name"});
        services.push_back(Log::getInstance());
        services.push_back(OtaService::get_instance(admin_pass));
        services.push_back(new RestServiceRobust(admin_acc, admin_pass, 80, ALL));
        strip = new PersistentLedStripService((RestService *) services.back(), false);
        services.push_back((Service *) strip);
        Log::println("Credentials: [%s:%s]", admin_acc, admin_pass);
        if (MDNS.begin(host_name)) {
            Log::println("Hostname: [%s]", host_name);
        }
        checked_free(admin_acc);
        checked_free(admin_pass);
        checked_free(host_name);

        attachInterrupt(CYCLE_BUTTON, ledStripCycleModesCallback, FALLING);
    }
    Log::println("Started.");
    delay(500);
}

void ledStripCycleModesCallback() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 200) {
        strip->set_mode(LED_STRIP_ANIM_MODE((strip->get_mode() + 1) % (ANIMATION_3 + 1)));
    }
    last_interrupt_time = interrupt_time;
}

void loop() {
    for (auto &service : services) {
        service->cycle_routine();
        yield(); // WATCHDOG/WIFI feed
    }
}