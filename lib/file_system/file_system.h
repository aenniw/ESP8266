#ifndef WEMOS_D1_FILESYSTEM_H_
#define WEMOS_D1_FILESYSTEM_H_

#include <ArduinoJson.h>
#include <initializer_list>
#include <FS.h>

String get_file_content(const char *);

template<typename T>
T get_config_(JsonObject &json, std::initializer_list<const char *> *keys,
              std::initializer_list<const char *>::const_iterator i) {
    if (!json.containsKey(*i)) {
        return 0;
    } else if (keys->end() == i + 1) {
        return json[*i].is<T>() ? json[*i].as<T>() : 0;
    } else {
        return get_config_<T>(json[*i].asObject(), keys, i + 1);
    }
}

template<>
const char *get_config_(JsonObject &json, std::initializer_list<const char *> *keys,
                        std::initializer_list<const char *>::const_iterator i) {
    if (!json.containsKey(*i)) {
        return NULL;
    } else if (keys->end() == i + 1) {
        if (!json[*i].is<const char *>())
            return NULL;
        const char *stack_array = json[*i].asString();
        char *heap_array = (char *) malloc(sizeof(char) * (strlen(stack_array) + 1));
        strcpy(heap_array, stack_array);
        return heap_array;
    } else {
        return get_config_<const char *>(json[*i].asObject(), keys, i + 1);
    }
}

template<typename T>
T get_config(const char *file, std::initializer_list<const char *> keys) {
    File configFile = SPIFFS.open(file, "r");
    if (!configFile) {
        Serial.println("No file");
        return std::is_pointer<T>::value ? NULL : 0;
    }
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    configFile.close();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (!json.success()) {
        return std::is_pointer<T>::value ? NULL : 0;
    } else {
        return get_config_<T>(json, &keys, keys.begin());
    }
}

template<typename T>
void set_config_(JsonObject &json, std::initializer_list<const char *> *keys,
                 std::initializer_list<const char *>::const_iterator i, T *value) {
    if (keys->end() == i + 1) {
        json[*i] = *value;
    } else {
        if (!json.containsKey(*i)) {
            json.createNestedObject(*i);
        } else {
            set_config_<T>(json[*i].asObject(), keys, i + 1, value);
        }
    }
}

template<typename T>
void set_config(const char *file, std::initializer_list<const char *> keys, T value) {
    File configFile = SPIFFS.open(file, "r");
    if (!configFile) return;
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    configFile.close();
    configFile = SPIFFS.open(file, "w");
    if (!configFile) return;
    set_config_(json, &keys, keys.begin(), &value);
    json.printTo(configFile);
    configFile.close();
}

#endif /* WEMOS_D1_FILESYSTEM_H_ */
