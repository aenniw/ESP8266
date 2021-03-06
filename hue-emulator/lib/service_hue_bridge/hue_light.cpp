#include "hue_light.h"

static const char *TEMPLATE_CONFIG_LIGHT = "/hue/l/cf-template.json";

LedLight::LedLight(LedStripService *l, const char *n, const uint8_t index)
        : HueLight(n, get_file_index_info(HUE_LIGHT, index, false), TEMPLATE_CONFIG_LIGHT) {
    ls = l;
    ls->set_animated_color_change(true);
    String unique_id = "AA:BB:CC:DD:EE:FF:00:11-";
    unique_id += index;
    ConfigJSON::set<const char *>(cf->name, {"uniqueid"}, unique_id.c_str());
    ConfigJSON::set<bool>(cf->name, {"state", "on"}, false);
    mark_for_reindex();
}

uint8_t LedLight::get_brightness() const {
    return ls->get_brightness();
}

void LedLight::set_color_cie(float x, float y) {
    HueLight::set_color_cie(x, y);
    ConfigJSON::clear_array(cf->name, {"state", "xy"});
    ConfigJSON::add_to_array<float>(cf->name, {"state", "xy"}, x);
    ConfigJSON::add_to_array<float>(cf->name, {"state", "xy"}, y);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "xy");
    mark_for_reindex();
}

void LedLight::set_color_ct(const uint32_t ct) {
    HueLight::set_color_ct(ct);
    ConfigJSON::set<uint32_t>(cf->name, {"state", "ct"}, ct);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "ct");
    mark_for_reindex();
}

void LedLight::set_color_rgb(const uint8_t _r, const uint8_t _g, const uint8_t _b) {
    ls->set_rgb(_r, _g, _b);
    HueLight::set_color_rgb(_r, _g, _b);
}

void LedLight::set_state(const bool s) {
    if (s) {
        ls->set_rgb(r, g, b);
    } else ls->set_rgb(0, 0, 0);
    ConfigJSON::set<bool>(cf->name, {"state", "on"}, s);
    mark_for_reindex();
}

void LedLight::set_hue(const uint16_t h) {
    ls->set_hue(h);
    ConfigJSON::set<uint16_t>(cf->name, {"state", "hue"}, h);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "hs");
    mark_for_reindex();
}

void LedLight::set_brightness(const uint8_t b) {
    ls->set_brightness(b);
    ConfigJSON::set<uint8_t>(cf->name, {"state", "bri"}, b);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "hs");
    mark_for_reindex();
}

void LedLight::set_saturation(const uint8_t s) {
    ls->set_saturation(s);
    ConfigJSON::set<uint8_t>(cf->name, {"state", "sat"}, s);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "hs");
    mark_for_reindex();
}

void LedLight::set_transition(const uint16_t t) {
    ls->set_delay(uint16_t(t * 0.25f));
}