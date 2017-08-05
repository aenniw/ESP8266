#include "hue_light.h"

static const char *TEMPLATE_CONFIG_LIGHT = "/hue/l/cf-template.json";

LedLight::LedLight(LedStripService *l, const char *n, const uint8_t index) : HueLight(n) {
    config = get_file_index_info(HUE_LIGHT, index, false)->name;
    config_all = config;
    copy_file(TEMPLATE_CONFIG_LIGHT, config);
    String unique_id = "AA:BB:CC:DD:EE:FF:00:11-";
    unique_id += index;
    ConfigJSON::set<const char *>(config, {"name"}, name);
    ConfigJSON::set<const char *>(config, {"uniqueid"}, unique_id.c_str());
    ls = l;
}

uint16_t LedLight::get_hue() const {
    return ls->get_hue();
};

uint8_t LedLight::get_saturation() const {
    return ls->get_saturation();
};

String LedLight::get_state() const {
    if (ls->get_color())
        return "true";
    return "false";
}

uint8_t LedLight::get_brightness() const {
    return ls->get_brightness();
}

void LedLight::set_color_cie(float x, float y) {
    HueLight::set_color_cie(x, y);
    ConfigJSON::clear_array(config, {"state", "xy"});
    ConfigJSON::add_to_array<float>(config, {"state", "xy"}, x);
    ConfigJSON::add_to_array<float>(config, {"state", "xy"}, y);
}

void LedLight::set_color_rgb(const uint8_t r, const uint8_t g, const uint8_t b) {
    ls->set_color(r, g, b);
}

void LedLight::set_state(const bool s) {
    // TODO
    if (s)
        ls->set_color(255, 255, 255);
    else ls->set_color(0, 0, 0);
    ConfigJSON::set<bool>(config, {"state", "on"}, s);
}

void LedLight::set_hue(const uint16_t h) {
    ls->set_hue(h);
    ConfigJSON::set<uint16_t>(config, {"state", "hue"}, h);
}

void LedLight::set_brightness(const uint8_t b) {
    ls->set_brightness(b);
    ConfigJSON::set<uint8_t>(config, {"state", "bri"}, b);
}

void LedLight::set_saturation(const uint8_t s) {
    ls->set_saturation(s);
    ConfigJSON::set<uint8_t>(config, {"state", "sat"}, s);
}