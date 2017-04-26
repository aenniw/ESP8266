#include <file_system.h>

String ICACHE_FLASH_ATTR get_file_content(const char *file_name) {
    File file = SPIFFS.open(file_name, "r");
    if (!file) {
        Serial.println("Failed to open file");
        return "";
    }
    String data = file.readString();
    file.close();
    return data;
}

void set_wifi_config_reset(const bool flag) {
    ConfigJSON::set<bool>(CONFIG_GLOBAL_JSON, {"default-config", "config-restored"}, !flag);
}

const bool ICACHE_FLASH_ATTR wifi_config_reset() {
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

char * ICACHE_FLASH_ATTR ConfigJSON::get_string(JsonObject &json, std::initializer_list<const char *> *keys,
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

char *ConfigJSON::getString(const char *file, std::initializer_list<const char *> keys) {
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