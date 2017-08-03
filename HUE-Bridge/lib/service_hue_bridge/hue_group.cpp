#include "hue_group.h"

static const char *TEMPLATE_CONFIG_GROUP = "/hue/g/cf-template.json";
static const char *TEMPLATE_CONFIG_A_GROUP = "/hue/g/cf-a-template.json";

uint16_t HueLightGroup::get_hue() const { return hue; }

uint8_t HueLightGroup::get_saturation() const { return sat; }

uint8_t HueLightGroup::get_brightness() const { return bri; }

String HueLightGroup::get_state() const {
    if (state) return "true";
    return "false";
}

void HueLightGroup::for_each_light(LightCallback callback) {
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        if (lights[i] != NULL) {
            callback(lights[i]);
        }
    }
}

bool HueLightGroup::add_light(const uint8_t i, HueLight *l) {
    if (i > MAX_HUE_LIGHTS || l == NULL)
        return false;
    lights[i] = l;
    //ConfigJSON::set<bool>(config, {}, NULL);
    //ConfigJSON::set<bool>(config_all, {}, NULL);
    return true;
}

void HueLightGroup::clear_lights() {
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        lights[i] = NULL;
    }
    //ConfigJSON::set<bool>(config, {}, NULL);
    //ConfigJSON::set<bool>(config_all, {}, NULL);
}

void HueLightGroup::set_color_rgb(const uint8_t r, const uint8_t g, const uint8_t b) {
    for_each_light([=](HueLight *l) { l->set_color_rgb(r, g, b); });
}

void HueLightGroup::set_state(const bool s) {
    state = s;
    for_each_light([=](HueLight *l) { l->set_state(s); });
}

void HueLightGroup::set_hue(const uint16_t h) {
    hue = h;
    for_each_light([=](HueLight *l) { l->set_hue(h); });
}

void HueLightGroup::set_brightness(const uint8_t b) {
    bri = b;
    for_each_light([=](HueLight *l) { l->set_brightness(b); });
}

void HueLightGroup::set_saturation(const uint8_t s) {
    sat = s;
    for_each_light([=](HueLight *l) { l->set_saturation(s); });
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

HueGroup::HueGroup(HueLight **l) : HueLightGroup("All lights") {
    config = get_file_index_info(HUE_GROUP, 0, false)->name;
    config_all = get_file_index_info(HUE_GROUP, 0, true)->name;
    copy_file(TEMPLATE_CONFIG_GROUP, config);
    copy_file(TEMPLATE_CONFIG_A_GROUP, config_all);
    ConfigJSON::set<const char *>(config, {"name"}, name);
    set_type("other");
    global = true;
    lights = l;
}

HueGroup::HueGroup(const char *n, const char *t, const uint8_t index) : HueLightGroup(n) {
    config = get_file_index_info(HUE_GROUP, index, false)->name;
    config_all = get_file_index_info(HUE_GROUP, index, true)->name;
    copy_file(TEMPLATE_CONFIG_GROUP, config);
    copy_file(TEMPLATE_CONFIG_A_GROUP, config_all);
    ConfigJSON::set<const char *>(config, {"name"}, name);
    lights = new HueLight *[MAX_HUE_LIGHTS];
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        lights[i] = NULL;
    }
    set_type(t);
}

void HueGroup::set_type(const char *t) {
    if (t == NULL) return;
    checked_free(type);
    type = (char *) malloc(sizeof(char) * (strlen(t) + 1));
    strcpy(type, t);
}

bool HueGroup::add_light(const uint8_t id, HueLight *l) {
    if (global) return false;
    return HueLightGroup::add_light(id, l);
}

void HueGroup::clear_lights() {
    if (!global)
        HueLightGroup::clear_lights();
}

String HueGroup::get_config() const {
    return "{}";
}

String HueGroup::get_config_complex() const {
    return "{}";
}

void HueGroup::set_state(const bool s) {
    HueLightGroup::set_state(s);
    ConfigJSON::set<bool>(config_all, {"action", "on"}, s);
}

void HueGroup::set_hue(const uint16_t h) {
    HueLightGroup::set_hue(h);
    ConfigJSON::set<uint16_t>(config_all, {"action", "hue"}, h);
}

void HueGroup::set_brightness(const uint8_t b) {
    HueLightGroup::set_brightness(b);
    ConfigJSON::set<uint8_t>(config_all, {"action", "bri"}, b);
}

void HueGroup::set_saturation(const uint8_t s) {
    HueLightGroup::set_saturation(s);
    ConfigJSON::set<uint8_t>(config_all, {"action", "sat"}, s);
}

HueGroup::~HueGroup() {
    if (!global) {
        delete lights;
    }
    checked_free(type);
}