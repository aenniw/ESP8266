#include "service_hue_bridge.h"

static const char *SSDP_RESPONSE_TEMPLATE =
        "HTTP/1.1 200 OK\r\n"
                "EXT:\r\n"
                "CACHE-CONTROL: max-age=%u\r\n" // SSDP_INTERVAL
                "LOCATION: http://%s:%u/%s\r\n" // WiFi.localIP(), _port, _schemaURL
                "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n" // _modelName, _modelNumber
                "hue-bridgeid: %s\r\n"
                "ST: %s\r\n"  // _deviceType
                "USN: uuid:%s\r\n" // _uuid
                "\r\n";

static const char *SSDP_NOTIFY_TEMPLATE =
        "NOTIFY * HTTP/1.1\r\n"
                "HOST: 239.255.255.250:1900\r\n"
                "NTS: ssdp:alive\r\n"
                "CACHE-CONTROL: max-age=%u\r\n" // SSDP_INTERVAL
                "LOCATION: http://%s:%u/%s\r\n" // WiFi.localIP(), _port, _schemaURL
                "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n" // _modelName, _modelNumber
                "hue-bridgeid: %s\r\n"
                "NT: %s\r\n"  // _deviceType
                "USN: uuid:%s\r\n" // _uuid
                "\r\n";
static const char *CONFIG_FILE_NAME = "/hue/config.json";

static uint8_t parse_id(const String &uri, uint8_t o = 0);

static const WcUriTranslator OBJECTS_TRANSLATOR = [](String uri) -> String {
    const uint8_t id = parse_id(uri, 4);
    if (uri.indexOf("lights") >= 0) {
        return get_file_index_info(HUE_LIGHT, id, false)->name;
    } else if (uri.indexOf("scenes") >= 0) {
        return get_file_index_info(HUE_SCENE, id, true)->name;
    } else if (uri.indexOf("groups") >= 0) {
        return get_file_index_info(HUE_GROUP, id, true)->name;
    }
    return "";
};

static uint8_t parse_id(const String &uri, uint8_t o) {
    uint8_t offset = 0;
    for (; offset < uri.length() && o > 0; offset++) {
        if (uri.charAt(offset) == '/') o--;
    }
    uint8_t id = (uint8_t) strtol(uri.c_str() + offset, NULL, 10);
    return id;
}

static String success(const String value) {
    String msg = "{ \"success\": { \"";
    msg += value;
    msg += "\" }";
    return msg;
}

static String success(const String id, const String value) {
    String msg = "{ \"success\": { \"";
    msg += id;
    msg += "\": \"";
    msg += value;
    msg += "\" } }";
    return msg;
}

static String success_int(const String id, const int value) {
    String msg = "{ \"success\": { \"";
    msg += id;
    msg += "\": \"";
    msg += value;
    msg += "\" } }";
    return msg;
}

static String error(int type, String path, String description) {
    String msg = "[ { \"error\": { \"type\": ";
    msg += type;
    msg += ", \"/address\": \"";
    msg += path;
    msg += "\", \"/description\": \"";
    msg += description;
    msg += "\" } }]";
    return msg;
}

static String emptyJSON(String nop, String nop_) {
    return "{}";
}

void HueBridge::restore_groups() {
    for (uint8_t i = 1; i < MAX_HUE_GROUPS; i++) {
        FileIndex *info = get_file_index_info(HUE_GROUP, i, false);
        if (info == NULL || !SPIFFS.exists(info->name)) continue;
        char *name = ConfigJSON::getString(info->name, {"name"});
        groups[i] = new HueGroup(name, "Restored Group", i);
        groups[i]->set_bridge_lights(lights);
        checked_free(name);
    }
}

void HueBridge::restore_scenes() {
    for (uint8_t i = 0; i < MAX_HUE_SCENES; i++) {
        FileIndex *info = get_file_index_info(HUE_SCENE, i, false);
        if (info == NULL || !SPIFFS.exists(info->name)) continue;
        char *name = ConfigJSON::getString(info->name, {"name"});
        scenes[i] = new HueScene(name, i);
        scenes[i]->set_bridge_lights(lights);
        checked_free(name);
    }
}

HueBridge::HueBridge(RestService *web_service) {
    bridgeIDString = WiFi.macAddress();
    bridgeIDString.replace(":", "");
    bridgeIDString = bridgeIDString.substring(0, 6) + "FFFE" + bridgeIDString.substring(6);

    ConfigJSON::set<String>(CONFIG_FILE_NAME, {"bridgeid"}, bridgeIDString);
    ConfigJSON::set<String>(CONFIG_FILE_NAME, {"mac"}, WiFi.macAddress());
    ConfigJSON::set<String>(CONFIG_FILE_NAME, {"ipaddress"}, WiFi.localIP().toString());
    ConfigJSON::set<String>(CONFIG_FILE_NAME, {"netmask"}, WiFi.subnetMask().toString());
    ConfigJSON::set<String>(CONFIG_FILE_NAME, {"gateway"}, WiFi.gatewayIP().toString());

    String udn = "uuid:2f402f80-da50-11e1-9b23-";
    udn += WiFi.macAddress();
    String ipString = "http://";
    ipString += WiFi.localIP().toString();
    ipString += ":80/";
    set_string("/hue/description.xml", "<UDN>", "</UDN>", udn.c_str());
    set_string("/hue/description.xml", "<serialNumber>", "</serialNumber>", WiFi.macAddress().c_str());
    set_string("/hue/description.xml", "<URLBase>", "</URLBase>", ipString.c_str());

    // the default group 0 is never listed
    groups[0] = new HueGroup("All lights", "other", 0);
    groups[0]->set_bridge_lights(lights);
    initialize_config(web_service);
    initialize_lights(web_service);
    initialize_groups(web_service);
    initialize_scenes(web_service);
    initialize_SSDP();

    restore_groups();
    restore_scenes();

    cycle_routine(); // reindex files
}

int16_t HueBridge::get_free_index(ConfigObject *const *ls, const uint8_t len, const uint8_t s) const {
    for (uint8_t i = s; i < len; i++) {
        const ConfigObject *l = ls[i];
        if (l == NULL) {
            return i;
        }
    }
    return -1;
}

String HueBridge::update_hue_lights(const String &arg, const String &uri, const String &path, HueLight *lights[],
                                    const uint16_t l) {
    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(arg);
    if (!json.success())
        return error(0, path, "Invalid request.");
    const uint8_t light_id = parse_id(uri, 4);
    if (light_id >= l || lights[light_id] == NULL)
        return error(0, path, "Resource not present.");
    const int bri = parseJSON<int>(json, "bri", -1),
            hue = parseJSON<int>(json, "hue", -1),
            sat = parseJSON<int>(json, "sat", -1),
            ct = parseJSON<int>(json, "ct", -1),
            transit_time = parseJSON<int>(json, "transitiontime", -1);
    if (json.containsKey("on") && json["on"].is<bool>()) {
        lights[light_id]->set_state(json["on"].as<bool>());
    }
    if (json.containsKey("xy") && json["xy"].is<JsonArray>()) {
        JsonArray &array = json["xy"].as<JsonArray>();
        lights[light_id]->set_color_cie(array[0].as<float>(), array[1].as<float>());
    } else if (ct >= 0) {
        // Conversion from mct to ct
        lights[light_id]->set_color_ct((uint32_t)(1000000 / ct));
    } else {
        if (bri >= 0) {
            lights[light_id]->set_brightness((uint8_t) bri);
        }
        if (hue >= 0) {
            lights[light_id]->set_hue((uint16_t) hue);
        }
        if (sat >= 0) {
            lights[light_id]->set_saturation((uint8_t) sat);
        }
    }
    if (transit_time >= 0) {
        lights[light_id]->set_transition((uint16_t) transit_time);
    }
    return "Updated.";
}

String
HueBridge::update_hue_groups(const String &arg, const String &uri, const String &path, HueLightGroup *lightGroups[],
                             const uint16_t l) {
    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(arg);
    if (!json.success())
        return error(0, path, "Invalid request.");
    const uint8_t id = parse_id(uri, 4);
    if (l >= l || lightGroups[id] == NULL)
        return error(0, path, "Resource not present.");
    const char *name = parseJSON<const char *>(json, "name", NULL);
    if (name != NULL) {
        lightGroups[id]->set_name(name);
    }
    if (json.containsKey("lights") && json["lights"].is<JsonArray>()) {
        JsonArray &array = json["lights"].as<JsonArray>();
        for (JsonArray::iterator it = array.begin(); it != array.end(); ++it) {
            const uint8_t light_id = it->as<uint8_t>();
            if (lights[light_id] == NULL) {
                return error(7, path, "Specified light does not exist.");
            }
            lightGroups[id]->add_light(light_id);
        }
    }
    return "Updated.";
}

void HueBridge::initialize_SSDP() {
    SSDP.begin();
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName("Philips hue clone");
    SSDP.setSerialNumber(WiFi.macAddress().c_str());
    SSDP.setURL("index.html");
    SSDP.setModelName("IpBridge");
    SSDP.setModelNumber("0.1");
    SSDP.setModelURL("http://www.meethue.com");
    SSDP.setManufacturer("Royal Philips Electronics");
    SSDP.setManufacturerURL("http://www.philips.com");
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setMessageFormatCallback([&](SSDPClass *ssdp, char *buffer, int buff_len,
                                      bool isNotify, int interval, char *modelName,
                                      char *modelNumber, char *uuid, char *deviceType,
                                      uint32_t ip, uint16_t port, char *schemaURL) {
        if (isNotify) {
            return snprintf(buffer, buff_len, SSDP_NOTIFY_TEMPLATE, interval, WiFi.localIP().toString().c_str(), port,
                            schemaURL, modelName, modelNumber, bridgeIDString.c_str(), deviceType, uuid);
        } else {
            return snprintf(buffer, buff_len, SSDP_RESPONSE_TEMPLATE, interval, WiFi.localIP().toString().c_str(),
                            port, schemaURL, modelName, modelNumber, bridgeIDString.c_str(), deviceType, uuid);
        }
    });
}

void HueBridge::initialize_config(RestService *web_service) {
    web_service->add_handler_file("/description.xml", HTTP_GET, RESP_TEXT, "/hue/description.xml", false);
    web_service->add_handler_wc_file("/api/+/config?", HTTP_GET, RESP_TEXT, "/hue/config.json", false);
    // TODO: implement?
    web_service->add_handler_wc("/api/+/config?", HTTP_PUT, RESP_TEXT, [this](String arg, String uri) -> String {
        return "[ { \"success\": {} }]";
    });
    web_service->add_handler_file("/api/config", HTTP_GET, RESP_TEXT, "/hue/config.json", false);
    web_service->add_handler_wc_stream("/api/+?", HTTP_GET, RESP_TEXT,
                                       new HueObjectsConfigStream(new HueObjectConfigStream(HUE_LIGHT),
                                                                  new HueObjectConfigStream(HUE_GROUP),
                                                                  new HueObjectConfigStream(HUE_SCENE)));
    // On the real bridge, the link button on the bridge must have been recently pressed for the command to execute successfully.
    // We try to execute successfully regardless of a button for now.
    web_service->add_handler_wc("/api?", HTTP_ANY, RESP_TEXT, [this](String arg, String uri) -> String {
        return "[" + success("username", "api") + "]";
    });
    // TODO: support later?
    web_service->add_handler_wc("/api/+/schedules?", HTTP_GET, RESP_TEXT, emptyJSON);
    web_service->add_handler_wc("/api/+/rules?", HTTP_GET, RESP_TEXT, emptyJSON);
    web_service->add_handler_wc("/api/+/sensors?", HTTP_GET, RESP_TEXT, emptyJSON);
}

void HueBridge::initialize_lights(RestService *web_service) {
    web_service->add_handler_wc("/api/+/lights/new?", HTTP_GET, RESP_TEXT, [this](String arg, String uri) -> String {


        return "{\"lastscan\": \"2000-10-29T12:00:00\" }";
    });
    web_service->add_handler_wc("/api/+/lights?", HTTP_POST, RESP_TEXT, [this](String arg, String uri) -> String {
        scanning = true;
        return "[" + success("/lights", "Searching for new devices") + "]";
    });
    web_service->add_handler_wc_stream("/api/+/lights?", HTTP_GET, RESP_TEXT, new HueObjectConfigStream(HUE_LIGHT));
    web_service->add_handler_wc_file("/api/+/lights/+?", HTTP_GET, RESP_JSON, OBJECTS_TRANSLATOR, false);
    web_service->add_handler_wc("/api/+/lights/+?", HTTP_PUT, RESP_TEXT, [this](String arg, String uri) -> String {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success())
            return error(0, "/lights/*", "Invalid request.");
        HueLight *light = get_light(parse_id(uri, 4));
        if (light == NULL)
            return error(0, "/lights/*", "Light not present.");
        const char *name = parseJSON<const char *>(json, "name", NULL);
        if (name != NULL) {
            light->set_name(name);
            return "[" + success("/lights/*", name) + "]";
        }
        return "[]";
    });
    web_service->add_handler_wc("/api/+/lights/+/state?", HTTP_PUT, RESP_TEXT, [this](String a, String u) -> String {
        return update_hue_lights(a, u, "/lights/*/state", lights, MAX_HUE_LIGHTS);
    });
}

void HueBridge::initialize_groups(RestService *web_service) {
    web_service->add_handler_wc_stream("/api/+/groups?", HTTP_GET, RESP_TEXT, new HueObjectConfigStream(HUE_GROUP));
    web_service->add_handler_wc("/api/+/groups?", HTTP_POST, RESP_TEXT, [this](String arg, String uri) -> String {
        StaticJsonBuffer<1024> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success())
            return error(0, "/groups", "Invalid request.");
        const char *name = parseJSON<const char *>(json, "name", NULL),
                *type = parseJSON<const char *>(json, "type", NULL);
        if (json.containsKey("lights") && json["lights"].is<JsonArray>() &&
            name != NULL && type != NULL) {
            JsonArray &array = json["lights"].as<JsonArray>();
            const int8_t group_id = add_group(name, type);
            if (group_id < 0) {
                return error(0, "/groups", "Group cannot be created.");
            }
            HueGroup *group = get_group((uint8_t) group_id);
            for (JsonArray::iterator it = array.begin(); it != array.end(); ++it) {
                const uint8_t light_id = it->as<uint8_t>();
                if (lights[light_id] == NULL) {
                    delete_group((uint8_t) group_id);
                    return error(7, "/groups", "Specified light does not exist.");
                }
                group->add_light(light_id);
            }
            return "[" + success_int("id", group_id) + "]";
        }
        return error(0, "/groups", "Insufficient arguments.");
    });
    web_service->add_handler_wc_file("/api/+/groups/+?", HTTP_GET, RESP_JSON, OBJECTS_TRANSLATOR, false);
    web_service->add_handler_wc("/api/+/groups/+?", HTTP_PUT, RESP_TEXT, [this](String arg, String uri) -> String {
        return update_hue_groups(arg, uri, "/groups/*", (HueLightGroup **) groups, MAX_HUE_GROUPS);
    });
    web_service->add_handler_wc("/api/+/groups/+?", HTTP_DELETE, RESP_TEXT, [this](String arg, String uri) -> String {
        const uint8_t id = parse_id(uri, 4);
        if (delete_group(id)) {
            return "[" + success("/groups/*", "deleted") + "]";
        }
        return error(0, "/groups/*", "Group not present.");
    });
    web_service->add_handler_wc("/api/+/groups/+/action?", HTTP_PUT, RESP_TEXT, [this](String a, String u) -> String {
        return update_hue_lights(a, u, "/groups/*/action", (HueLight **) groups, MAX_HUE_GROUPS);
    });
}

void HueBridge::initialize_scenes(RestService *web_service) {
    web_service->add_handler_wc_stream("/api/+/scenes?", HTTP_GET, RESP_TEXT, new HueObjectConfigStream(HUE_SCENE));
    web_service->add_handler_wc("/api/+/scenes?", HTTP_POST, RESP_TEXT, [this](String arg, String uri) -> String {
        StaticJsonBuffer<1024> jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(arg);
        if (!json.success())
            return error(0, "/scenes", "Invalid request.");
        const char *name = parseJSON<const char *>(json, "name", NULL);
        if (json.containsKey("lights") && json["lights"].is<JsonArray>() && name != NULL) {
            JsonArray &array = json["lights"].as<JsonArray>();
            const int8_t scene_id = add_scene(name);
            if (scene_id < 0) {
                return error(0, "/scenes", "Group cannot be created.");
            }
            HueScene *scene = get_scene((uint8_t) scene_id);
            for (JsonArray::iterator it = array.begin(); it != array.end(); ++it) {
                const uint8_t light_id = it->as<uint8_t>();
                if (lights[light_id] == NULL) {
                    delete_scene((uint8_t) scene_id);
                    return error(7, "/scenes", "Specified light does not exist.");
                }
                scene->add_light(light_id);
            }
            if (json.containsKey("recycle") && json["recycle"].is<bool>()) {
                scene->set_recycle(parseJSON<bool>(json, "recycle"));
            }
            return "[" + success_int("id", scene_id) + "]";
        }
        return error(0, "/scenes", "Insufficient arguments.");
    });
    web_service->add_handler_wc_file("/api/+/scenes/+?", HTTP_GET, RESP_JSON, OBJECTS_TRANSLATOR, false);
    web_service->add_handler_wc("/api/+/scenes/+?", HTTP_PUT, RESP_TEXT, [this](String arg, String uri) -> String {
        return update_hue_groups(arg, uri, "/scenes/*", (HueLightGroup **) scenes, MAX_HUE_SCENES);
        //if (json.containsKey("storelightstate") && json["storelightstate"].is<bool>()) {
            // TODO: If set, the lightstates of the lights in the scene will be overwritten by the current state of the lights.
            // Can also be used in combination with transitiontime to update the transition time of a scene.
        //}
    });
    web_service->add_handler_wc("/api/+/scenes/+?", HTTP_DELETE, RESP_TEXT, [this](String arg, String uri) -> String {
        const uint8_t id = parse_id(uri, 4);
        if (delete_scene(id)) {
            return "[" + success("/scenes/* deleted") + "]";
        }
        return error(0, "/scenes/*", "Scene not present.");
    });
    web_service->add_handler_wc("/api/+/scenes/+/lights/+/state?", HTTP_PUT, RESP_TEXT,
                                [this](String a, String u) -> String {
                                    return update_hue_lights(a, u, "/scenes/*/lights/*/states", (HueLight **) scenes,
                                                             MAX_HUE_SCENES);
                                });
}

HueLight *HueBridge::get_light(const uint8_t i) const {
    if (i >= MAX_HUE_LIGHTS) return NULL;
    return lights[i];
}

HueGroup *HueBridge::get_group(const uint8_t i) const {
    if (i >= MAX_HUE_GROUPS) return NULL;
    return groups[i];
}

HueScene *HueBridge::get_scene(const uint8_t i) const {
    if (i >= MAX_HUE_SCENES) return NULL;
    return scenes[i];
}

int8_t HueBridge::add_light(LedStripService *l) {
    const int16_t i = get_free_index((ConfigObject **) lights, MAX_HUE_LIGHTS);
    if (l == NULL || i < 0) return -1;
    lights[i] = new LedLight(l, "Hue Light", (uint8_t) i);
    groups[0]->add_light((uint8_t) i);
    return (int8_t) i;
}

int8_t HueBridge::add_light(IPAddress address) {
    const int16_t i = get_free_index((ConfigObject **) lights, MAX_HUE_LIGHTS);
    if (i < 0) return -1;
    lights[i] = new RemoteLedLight(address, "Hue Light", (uint8_t) i);
    groups[0]->add_light((uint8_t) i);
    return (int8_t) i;
}

bool HueBridge::delete_light(const uint8_t id) {
    if (id >= MAX_HUE_LIGHTS || lights[id] == NULL) return false;
    delete lights[id];
    lights[id] = NULL;
    return true;
}

int8_t HueBridge::add_group(const char *n, const char *t) {
    const int16_t i = get_free_index((ConfigObject **) groups, MAX_HUE_GROUPS);
    if (n == NULL || t == NULL || i < 0) return -1;
    groups[i] = new HueGroup(n, t, (uint8_t) i);
    groups[i]->set_bridge_lights(lights);
    return (int8_t) i;
}

bool HueBridge::delete_group(const uint8_t id) {
    if (id >= MAX_HUE_GROUPS || groups[id] == NULL || id == 0) return false;
    delete groups[id];
    groups[id] = NULL;
    return true;
}

int8_t HueBridge::add_scene(const char *n) {
    const int16_t i = get_free_index((ConfigObject **) scenes, MAX_HUE_SCENES);
    if (n == NULL || i < 0) return -1;
    scenes[i] = new HueScene(n, (uint8_t) i);
    scenes[i]->set_bridge_lights(lights);
    return (int8_t) i;
}

bool HueBridge::delete_scene(const uint8_t id) {
    if (id >= MAX_HUE_SCENES || scenes[id] == NULL || id == 0) return false;
    delete scenes[id];
    scenes[id] = NULL;
    return true;
}


static String sendGET(IPAddress &ipAddress, String uri) {
    HTTPClient client;
    String payload;

    client.begin(ipAddress.toString(), 80, uri);
    int httpCode = client.GET();
    if (httpCode > 0) {
        Log::println("[HTTP] GET... code: %d", httpCode);

        if (httpCode == HTTP_CODE_OK) {
            payload = client.getString();
            Log::println(payload);
        }
    } else {
        Log::println("[HTTP] GET... failed, error: %s", client.errorToString(httpCode).c_str());
    }
    client.end();
    return payload;
}

void HueBridge::cycle_routine() {
    if (scanning) {
        scanning = !scanning;
        const int n = MDNS.queryService("hue", "tcp");
        for (int i = 0; i < n; i++) {
            IPAddress address = MDNS.IP(i);
            String resp = sendGET(address, "/led-strip/get-config");
            if (resp.length() > 0) {
                add_light(address);
            }
        }
    }
    MDNS.update();
    reindex_all();
    resend_queries();
}

HueBridge::~HueBridge() {
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        if (lights[i] != NULL) delete lights[i];
    }
    for (uint8_t i = 0; i < MAX_HUE_GROUPS; i++) {
        if (groups[i] != NULL) delete groups[i];
    }
    for (uint8_t i = 0; i < MAX_HUE_SCENES; i++) {
        if (scenes[i] != NULL) delete scenes[i];
    }
}
