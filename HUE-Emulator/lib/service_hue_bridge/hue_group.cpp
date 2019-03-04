#include "hue_group.h"

static const char *TEMPLATE_CONFIG_GROUP = "/hue/g/cf-template.json";
static const char *TEMPLATE_CONFIG_A_GROUP = "/hue/g/cf-a-template.json";

HueGroup::HueGroup(const char *n, const char *t, const uint8_t index)
        : HueLightGroup(n, get_file_index_info(HUE_GROUP, index, false),
                        get_file_index_info(HUE_GROUP, index, true),
                        TEMPLATE_CONFIG_GROUP, TEMPLATE_CONFIG_A_GROUP) {
    ConfigJSON::set<bool>(cfa->name, {"action", "on"}, false);
    mark_for_reindex();
}

void HueGroup::set_state(const bool s) {
    HueLightGroup::set_state(s);
    ConfigJSON::set<bool>(cfa->name, {"action", "on"}, s);
    mark_for_reindex();
}

void HueGroup::set_color_cie(float x, float y) {
    HueLightGroup::set_color_cie(x, y);
    ConfigJSON::clear_array(cfa->name, {"action", "xy"});
    ConfigJSON::add_to_array<float>(cfa->name, {"action", "xy"}, x);
    ConfigJSON::add_to_array<float>(cfa->name, {"action", "xy"}, y);
    mark_for_reindex();
}

void HueGroup::set_color_ct(const uint32_t ct) {
    HueLightGroup::set_color_ct(ct);
    ConfigJSON::set<uint32_t>(cfa->name, {"action", "ct"}, ct);
    mark_for_reindex();
}

void HueGroup::set_hue(const uint16_t h) {
    HueLightGroup::set_hue(h);
    ConfigJSON::set<uint16_t>(cfa->name, {"action", "hue"}, h);
    mark_for_reindex();
}

void HueGroup::set_brightness(const uint8_t b) {
    HueLightGroup::set_brightness(b);
    ConfigJSON::set<uint8_t>(cfa->name, {"action", "bri"}, b);
    mark_for_reindex();
}

void HueGroup::set_saturation(const uint8_t s) {
    HueLightGroup::set_saturation(s);
    ConfigJSON::set<uint8_t>(cfa->name, {"action", "sat"}, s);
    mark_for_reindex();
}