#include "hue_group.h"

static const char *TEMPLATE_CONFIG_GROUP = "/hue/g/cf-template.json";
static const char *TEMPLATE_CONFIG_A_GROUP = "/hue/g/cf-a-template.json";

HueGroup::HueGroup(const char *n, const char *t, const uint8_t index) : HueLightGroup(n) {
    cf = get_file_index_info(HUE_GROUP, index, false);
    cfa = get_file_index_info(HUE_GROUP, index, true);

    copy_file(TEMPLATE_CONFIG_GROUP, cf->name);
    copy_file(TEMPLATE_CONFIG_A_GROUP, cfa->name);
    ConfigJSON::set<const char *>(cf->name, {"name"}, name);
    lights = new HueLight *[MAX_HUE_LIGHTS];
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        lights[i] = NULL;
    }
    set_type(t);
    mark_for_reindex();
}

void HueGroup::set_type(const char *t) {
    if (t == NULL) return;
    checked_free(type);
    type = (char *) malloc(sizeof(char) * (strlen(t) + 1));
    strcpy(type, t);
}

bool HueGroup::add_light(const uint8_t id, HueLight *l) {
    return HueLightGroup::add_light(id, l);
}

void HueGroup::clear_lights() {
    HueLightGroup::clear_lights();
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

void HueGroup::set_hue(const uint16_t h) {
    HueLightGroup::set_hue(h);
    ConfigJSON::set<uint16_t>(cfa->name, {"action", "hue"}, h);
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

HueGroup::~HueGroup() {
    delete lights;
    checked_free(type);
}