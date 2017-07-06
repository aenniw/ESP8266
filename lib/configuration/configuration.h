#ifndef ESP8266_PROJECTS_ROOT_CONFIGURATION_H
#define ESP8266_PROJECTS_ROOT_CONFIGURATION_H

#include <service_rest.h>
#include <service_led_strip.h>
#include <file_system.h>
#include <ArduinoJson.h>

typedef enum {
    ALL = 0xFFFFFFFF,
    CALLBACKS_WIFI = 0x1,
    CALLBACKS_SYSTEM = 0x2,
    LOGGING = 0x4,
    HTML_DIGITAL_IO = 0x8,
    HTML_COMMON_FILES = 0x10,
    HTML_ADMIN_FILES = 0x20,
    HTML_STATUS_FILES = 0x40,
    HTML_LED_STRIP_FILES = 0x80,
    HTML_ALL_FILES = 0xF8
} REST_INIT;

RestService *init_rest(RestService *, REST_INIT);

RestService *init_rest(RestService *web_service, LedStripService *led_service, REST_INIT scope);

#endif //ESP8266_PROJECTS_ROOT_CONFIGURATION_H
