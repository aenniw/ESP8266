#include <file_system.h>

void set_wifi_config_reset(const bool flag) {
    ConfigJSON::set<bool>(CONFIG_GLOBAL_JSON, {"default-config", "config-restored"}, !flag);
}

void config_reset_check() {
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
}

bool get_wifi_config_reset() {
    return ConfigJSON::get<bool>(CONFIG_GLOBAL_JSON, {"default-config", "config-restored"});
}

const bool ICACHE_FLASH_ATTR wifi_config_reset() {
    if (!get_wifi_config_reset()) {
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
            default:
                break;
        }
        // Toggle reset flag
        set_wifi_config_reset(false);
        // Free allocated resources
        checked_free(wifi_ap_ssid);
        checked_free(wifi_ap_pass);
        checked_free(wifi_sta_ssid);
        checked_free(wifi_sta_pass);
        return true;
    }
    return false;
}

char** ICACHE_FLASH_ATTR ConfigJSON::get_string_array(JsonObject &json, std::initializer_list<const char *> *keys,
                                    std::initializer_list<const char *>::const_iterator i) {
    if (!json.containsKey(*i)) {
        return NULL;
    } else if (keys->end() == i + 1) {
        if (json[*i].is<JsonArray>() && json[*i].as<JsonArray>().size()) {
            JsonArray &json_array = json[*i].as<JsonArray>();
            char **array = new char *[json_array.size()];
            for (uint16_t o = 0; o < json_array.size(); o++) {
                const char *stack = json_array.get<const char *>(o);
                array[o] = (char *) malloc(sizeof(char) * (strlen(stack) + 1));
                strcpy(array[o], stack);
            }
            return array;
        } else {
            return NULL;
        }
    } else {
        return get_string_array(json[*i].as<JsonObject>(), keys, i + 1);
    }
}

uint32_t ICACHE_FLASH_ATTR ConfigJSON::get_array_len(JsonObject &json, std::initializer_list<const char *> *keys,
                                   std::initializer_list<const char *>::const_iterator i) {
    if (!json.containsKey(*i)) {
        return 0;
    } else if (keys->end() == i + 1) {
        if (json[*i].is<JsonArray>()) {
            return json[*i].as<JsonArray>().size();
        } else {
            return 0;
        }
    } else {
        return get_array_len(json[*i].as<JsonObject>(), keys, i + 1);
    }
}

char *ICACHE_FLASH_ATTR ConfigJSON::get_string(JsonObject &json, std::initializer_list<const char *> *keys,
                                               std::initializer_list<const char *>::const_iterator i) {
    if (!json.containsKey(*i)) {
        return NULL;
    } else if (keys->end() == i + 1) {
        if (json[*i].is<const char *>()) {
            const char *stack = json[*i].as<const char *>();
            char *heap = (char *) malloc(sizeof(char) * (strlen(stack) + 1));
            strcpy(heap, stack);
            return heap;
        } else {
            return NULL;
        }
    } else {
        return get_string(json[*i].as<JsonObject>(), keys, i + 1);
    }
}

char *ICACHE_FLASH_ATTR ConfigJSON::getString(const char *file, std::initializer_list<const char *> keys) {
    File configFile = SPIFFS.open(file, "r");
    if (!configFile) {
        Log::println("No file");
        return NULL;
    }
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    configFile.close();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (!json.success()) {
        return NULL;
    } else {
        return get_string(json, &keys, keys.begin());
    }
}

uint32_t ICACHE_FLASH_ATTR ConfigJSON::get_array_len(const char *file, std::initializer_list<const char *> keys) {
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
        return get_array_len(json, &keys, keys.begin());
    }
}

char** ICACHE_FLASH_ATTR ConfigJSON::get_string_array(const char *file, std::initializer_list<const char *> keys) {
    File configFile = SPIFFS.open(file, "r");
    if (!configFile) {
        Log::println("No file");
        return NULL;
    }
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    configFile.close();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (!json.success()) {
        return NULL;
    } else {
        return get_string_array(json, &keys, keys.begin());
    }
}

void ICACHE_FLASH_ATTR ConfigJSON::del(const char *file, std::initializer_list<const char *> keys) {
    File configFile = SPIFFS.open(file, "r");
    if (!configFile) return;
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    configFile.close();
    configFile = SPIFFS.open(file, "w");
    if (!configFile) return;
    del_default(json, &keys, keys.begin());
    json.printTo(configFile);
    configFile.close();
}

void ICACHE_FLASH_ATTR ConfigJSON::del_default(JsonObject &json, std::initializer_list<const char *> *keys,
                             std::initializer_list<const char *>::const_iterator i) {
    if (keys->end() == i + 1) {
        json.remove(*i);
    } else {
        if (!json.containsKey(*i)) {
            json.createNestedObject(*i);
        } else {
            del_default(json[*i].as<JsonObject>(), keys, i + 1);
        }
    }
}

bool ICACHE_FLASH_ATTR copy_file(const char *src_name, const char *dst_name, const bool overwrite) {
    if (!overwrite && SPIFFS.exists(dst_name)) return false;
    File dst = SPIFFS.open(dst_name, "w");
    if (!dst) return false;
    File src = SPIFFS.open(src_name, "r");
    if (!src) {
        dst.close();
        return false;
    }
    while (src.available()) {
        dst.write((uint8_t) src.read());
    }
    src.close();
    dst.flush();
    dst.close();
    Log::println("File %s copied to %s", src_name, dst_name);
    return true;
}