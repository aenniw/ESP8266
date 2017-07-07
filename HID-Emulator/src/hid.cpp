#include <Arduino.h>
#include <vector>

#include <commons.h>
#include <file_system.h>
#include <service_rest_robust.h>
#include <service_ota.h>
#include <service_hid.h>

#define JS_HID "/js/hid.js"
#define HTML_HID "/html/hid.html"

std::vector<Service *> services;

void ICACHE_FLASH_ATTR setup() {
#ifdef __DEBUG__
    Log::init();
#endif
    if (!SPIFFS.begin()) {
#ifdef __DEBUG__
        Log::println("SPIFFS failed to initialize flash corrupted?");
#endif
        ESP.restart();
    }
    {   // Services setup
        char *admin_acc = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-acc"}),
                *admin_pass = ConfigJSON::getString(CONFIG_GLOBAL_JSON, {"rest-pass"});
#ifdef __DEBUG__
        services.push_back(Log::getInstance());
#endif
        services.push_back(HID::getInstance());
        //services.push_back(HID::getInstance(admin_acc, admin_pass));
        services.push_back(OtaService::get_instance(admin_pass));
        RestService *rest_service = new RestServiceRobust(admin_acc, admin_pass, 80,
                                                          (REST_INIT) (CALLBACKS_SYSTEM | CALLBACKS_WIFI |
                                                                       HTML_ALL_FILES));
        rest_service->add_handler_file(HTML_HID, HTTP_ANY, RESP_HTML, HTML_HID
                ".gz", true);
        rest_service->add_handler_file(JS_HID, HTTP_ANY, RESP_JS, JS_HID
                ".gz", true);
        services.push_back((Service *&&) rest_service);
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
