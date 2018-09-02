#ifndef WEMOS_D1_FILESYSTEM_H_
#define WEMOS_D1_FILESYSTEM_H_

#include <FS.h>
#include <ArduinoJson.h>
#include <initializer_list>
#include <service_log.h>
//#include <EEPROM.h>

#define CONFIG_IO_JSON "/json/config-io.json"

void ICACHE_FLASH_ATTR set_wifi_config_reset(const bool);
bool ICACHE_FLASH_ATTR get_wifi_config_reset();

const bool ICACHE_FLASH_ATTR wifi_config_reset();

class ConfigEEPROM
{
    // TODO: implement EEPROM wrapper for wiring complex data structures SEE: https://github.com/esp8266/Arduino/tree/master/libraries/EEPROM/examples
};

template <typename T>
static T _get_default(JsonObject &json, std::initializer_list<const char *> *keys,
                      std::initializer_list<const char *>::const_iterator i)
{
    if (!json.containsKey(*i))
    {
        return 0;
    }
    else if (keys->end() == i + 1)
    {
        return json[*i].is<T>() ? json[*i].as<T>() : 0;
    }
    else
    {
        return _get_default<T>(json[*i].as<JsonObject>(), keys, i + 1);
    }
}

template <typename T>
static void _set_default(JsonObject &json, std::initializer_list<const char *> *keys,
                         std::initializer_list<const char *>::const_iterator i, T *value)
{
    if (keys->end() == i + 1)
    {
        json[*i] = *value;
    }
    else
    {
        if (!json.containsKey(*i))
        {
            json.createNestedObject(*i);
        }
        else
        {
            _set_default<T>(json[*i].as<JsonObject>(), keys, i + 1, value);
        }
    }
}

template <typename T>
static void _add_to_array(JsonObject &json, std::initializer_list<const char *> *keys,
                          std::initializer_list<const char *>::const_iterator i, T *value)
{
    if (keys->end() == i + 1)
    {
        json[*i].as<JsonArray>().add(*value);
        ;
    }
    else
    {
        if (!json.containsKey(*i))
        {
            json.createNestedObject(*i);
        }
        else
        {
            _add_to_array<T>(json[*i].as<JsonObject>(), keys, i + 1, value);
        }
    }
}

template <typename T>
static T *_get_array(JsonObject &json, std::initializer_list<const char *> *keys,
                     std::initializer_list<const char *>::const_iterator i)
{
    if (keys->end() == i + 1)
    {
        if (!json[*i].is<JsonArray>())
        {
            return NULL;
        }
        JsonArray &json_array = json[*i].as<JsonArray>();
        T *array = new T[json_array.size()];
        for (uint16_t o = 0; o < json_array.size(); o++)
        {
            array[o] = json_array.get<T>(o);
        }
        return array;
    }
    else
    {
        if (!json.containsKey(*i))
        {
            return NULL;
        }
        else
        {
            _get_array<T>(json[*i].as<JsonObject>(), keys, i + 1);
        }
    }
}

template <typename T>
static T _get(const char *file, std::initializer_list<const char *> keys)
{
    File configFile = SPIFFS.open(file, "r");
    if (!configFile)
    {
        Log::println("No file");
        return 0;
    }
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    configFile.close();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (!json.success())
    {
        return 0;
    }
    else
    {
        return _get_default<T>(json, &keys, keys.begin());
    }
}

template <typename T>
static bool _set(const char *file, std::initializer_list<const char *> keys, T value)
{
    File configFile = SPIFFS.open(file, "r");
    if (!configFile)
        return false;
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    configFile.close();
    configFile = SPIFFS.open(file, "w");
    if (!configFile)
        return false;
    _set_default(json, &keys, keys.begin(), &value);
    json.printTo(configFile);
    configFile.close();
    return true;
}

template <typename T>
static bool _add_to_array(const char *file, std::initializer_list<const char *> keys, T value)
{
    File configFile = SPIFFS.open(file, "r");
    if (!configFile)
        return false;
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    configFile.close();
    configFile = SPIFFS.open(file, "w");
    if (!configFile)
        return false;
    _add_to_array(json, &keys, keys.begin(), &value);
    json.printTo(configFile);
    configFile.close();
    return true;
}

template <typename T>
static T *_get_array(const char *file, std::initializer_list<const char *> keys)
{
    File configFile = SPIFFS.open(file, "r");
    if (!configFile)
        return NULL;
    std::unique_ptr<char[]> buf(new char[configFile.size()]);
    configFile.readBytes(buf.get(), configFile.size());
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    configFile.close();
    configFile = SPIFFS.open(file, "w");
    if (!configFile)
        return NULL;
    T *array = _get_array<T>(json, &keys, keys.begin());
    json.printTo(configFile);
    configFile.close();
    return array;
}

class ConfigJSON
{
  private:
    static void ICACHE_FLASH_ATTR del_default(JsonObject &json, std::initializer_list<const char *> *keys,
                                              std::initializer_list<const char *>::const_iterator i);

    static char *ICACHE_FLASH_ATTR get_string(JsonObject &json, std::initializer_list<const char *> *keys,
                                              std::initializer_list<const char *>::const_iterator i);

    static char **ICACHE_FLASH_ATTR get_string_array(JsonObject &json, std::initializer_list<const char *> *keys,
                                                     std::initializer_list<const char *>::const_iterator i);

    static uint32_t ICACHE_FLASH_ATTR get_array_len(JsonObject &json, std::initializer_list<const char *> *keys,
                                                    std::initializer_list<const char *>::const_iterator i);

  public:
    template <typename T>
    static T ICACHE_FLASH_ATTR get(const char *file, std::initializer_list<const char *> keys)
    {
        return _get<T>(file, keys);
    }

    template <typename T>
    static bool ICACHE_FLASH_ATTR set(const char *file, std::initializer_list<const char *> keys, T value)
    {
        return _set<T>(file, keys, value);
    }

    template <typename T>
    static bool ICACHE_FLASH_ATTR
    add_to_array(const char *file, std::initializer_list<const char *> keys, T value)
    {
        return _add_to_array<T>(file, keys, value);
    }

    template <typename T>
    static T *ICACHE_FLASH_ATTR
    get_array(const char *file, std::initializer_list<const char *> keys)
    {
        return _get_array<T>(file, keys);
    }

    static bool ICACHE_FLASH_ATTR
    clear_array(const char *file, std::initializer_list<const char *> keys)
    {
        StaticJsonBuffer<10> buffer;
        return _set<JsonArray &>(file, keys, buffer.createArray());
    }

    static void ICACHE_FLASH_ATTR del(const char *file, std::initializer_list<const char *> keys);

    static char *ICACHE_FLASH_ATTR getString(const char *file, std::initializer_list<const char *> keys);

    static char **ICACHE_FLASH_ATTR get_string_array(const char *file, std::initializer_list<const char *> keys);

    static uint32_t ICACHE_FLASH_ATTR get_array_len(const char *file, std::initializer_list<const char *> keys);
};

bool ICACHE_FLASH_ATTR copy_file(const char *src_name, const char *dst_name, const bool overwrite = false);

#endif /* WEMOS_D1_FILESYSTEM_H_ */
