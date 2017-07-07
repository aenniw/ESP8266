#ifndef ESP8266_PROJECTS_ROOT_COMMONS_JSON_H
#define ESP8266_PROJECTS_ROOT_COMMONS_JSON_H

#include <ArduinoJson.h>

template<typename T>
static T parseJSON(JsonObject &json, const char *arg, T default_value = 0) {
    if (json.containsKey(arg) && json[arg].is<T>()) {
        return json[arg].as<T>();
    }
    return default_value;
}

#endif //ESP8266_PROJECTS_ROOT_COMMONS_JSON_H
