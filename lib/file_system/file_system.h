#ifndef WEMOS_D1_FILESYSTEM_H_
#define WEMOS_D1_FILESYSTEM_H_

#include <logger.h>
#include <ArduinoJson.h>
#include <initializer_list>
#include <FS.h>
#include <EEPROM.h>

#define CONFIG_GLOBAL_JSON "/json/config-global.json"
#define HTML_INDEX "/index.html"
#define HTML_LOGIN "/login.html"
#define HTML_ADMINISTRATION "/html/administration.html"
#define HTML_STATUS "/html/status.html"
#define HTML_WIFI "/html/wifi.html"
#define HTML_LOG "/html/log.html"
#define CSS_COMMON "/css/common_style.css"
#define CSS_INDEX "/css/index_style.css"
#define CSS_LOGIN "/css/login_style.css"
#define JS_COMMON "/js/common.js"
#define JS_LOG "/js/log.js"
#define JS_ADMINISTRATION "/js/administration.js"
#define JS_STATUS "/js/status.js"

String ICACHE_FLASH_ATTR get_file_content(const char *);

class ConfigEEPROM {
    // TODO: implement EEPROM wrapper for wiring complex data structures SEE: https://github.com/esp8266/Arduino/tree/master/libraries/EEPROM/examples
};

class ConfigJSON {
private:
    template<typename T>
    static T ICACHE_FLASH_ATTR get_default(JsonObject &json, std::initializer_list<const char *> *keys,
                                           std::initializer_list<const char *>::const_iterator i) {
        if (!json.containsKey(*i)) {
            return 0;
        } else if (keys->end() == i + 1) {
            return json[*i].is<T>() ? json[*i].as<T>() : 0;
        } else {
            return get_default<T>(json[*i].as<JsonObject>(), keys, i + 1);
        }
    }

    template<typename T>
    static void ICACHE_FLASH_ATTR set_default(JsonObject &json, std::initializer_list<const char *> *keys,
                                              std::initializer_list<const char *>::const_iterator i, T *value) {
        if (keys->end() == i + 1) {
            json[*i] = *value;
        } else {
            if (!json.containsKey(*i)) {
                json.createNestedObject(*i);
            } else {
                set_default<T>(json[*i].as<JsonObject>(), keys, i + 1, value);
            }
        }
    }

    static char *ICACHE_FLASH_ATTR get_string(JsonObject &json, std::initializer_list<const char *> *keys,
                                              std::initializer_list<const char *>::const_iterator i);

public:
    template<typename T>
    static T ICACHE_FLASH_ATTR get(const char *file, std::initializer_list<const char *> keys) {
        File configFile = SPIFFS.open(file, "r");
        if (!configFile) {
            Log::println("No file");
            return 0;
        }
        std::unique_ptr<char[]> buf(new char[configFile.size()]);
        configFile.readBytes(buf.get(), configFile.size());
        configFile.close();
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        if (!json.success()) {
            return 0;
        } else {
            return get_default<T>(json, &keys, keys.begin());
        }
    }

    template<typename T>
    static bool ICACHE_FLASH_ATTR set(const char *file, std::initializer_list<const char *> keys, T value) {
        File configFile = SPIFFS.open(file, "r");
        if (!configFile) return false;
        std::unique_ptr<char[]> buf(new char[configFile.size()]);
        configFile.readBytes(buf.get(), configFile.size());
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        configFile.close();
        configFile = SPIFFS.open(file, "w");
        if (!configFile) return false;
        set_default(json, &keys, keys.begin(), &value);
        json.printTo(configFile);
        configFile.close();
        return true;
    }

    static char *getString(const char *file, std::initializer_list<const char *> keys);
};

void set_wifi_config_reset(const bool);

const bool ICACHE_FLASH_ATTR wifi_config_reset();

#endif /* WEMOS_D1_FILESYSTEM_H_ */
