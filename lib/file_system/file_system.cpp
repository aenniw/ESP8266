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