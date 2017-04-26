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
    {   // Restore default config
        if (!ConfigJSON::get<bool>(CONFIG_GLOBAL_JSON, {"default-config", "config-restored"})) {
            Log::println("Restore default config");
            const WiFiMode_t wifi_mode = (WiFiMode_t) ConfigJSON::get<int>(CONFIG_GLOBAL_JSON,
                                                                           {"default-config", "wifi-mode"});
            WiFi.mode(wifi_mode);
            char *wifi_ap_ssid = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"default-config", "wifi-ap-ssid"}),
                    *wifi_ap_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"default-config", "wifi-ap-pass"}),
                    *wifi_sta_ssid = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"default-config", "wifi-sta-ssid"}),
                    *wifi_sta_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"default-config", "wifi-sta-pass"});
            switch (wifi_mode) {
                case WIFI_AP:
                    WiFi.softAP(wifi_ap_ssid, wifi_ap_pass);
                    break;
                case WIFI_AP_STA:
                    WiFi.softAP(wifi_ap_ssid, wifi_ap_pass);
                    WiFi.begin(wifi_sta_ssid, wifi_sta_pass);
                    break;
                case WIFI_STA:
                    WiFi.begin(wifi_sta_ssid, wifi_sta_pass);
                    break;
            }
            // Toggle reset flag
            ConfigJSON::set<bool>(CONFIG_GLOBAL_JSON, {"default-config", "config-restored"}, true);
            // Free allocated resources
            checked_free(wifi_ap_ssid);
            checked_free(wifi_ap_pass);
            checked_free(wifi_sta_ssid);
            checked_free(wifi_sta_pass);
        }
    }
    delay(500);
}

void loop() {
    for (std::vector<Service *>::iterator i = services.begin();
         i != services.end(); i++)
        (*i)->cycle_routine();
    yield(); // WATCHDOG/WIFI feed
}
