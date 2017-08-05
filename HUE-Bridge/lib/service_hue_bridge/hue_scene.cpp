#include "hue_scene.h"

static const char *TEMPLATE_CONFIG_SCENE = "/hue/s/cf-template.json";
static const char *TEMPLATE_CONFIG_A_SCENE = "/hue/s/cf-a-template.json";

HueScene::HueScene(const char *n, const uint8_t index) : HueLightGroup(n) {
    config = get_file_index_info(HUE_LIGHT, index, false)->name;
    config_all = get_file_index_info(HUE_LIGHT, index, true)->name;

    copy_file(TEMPLATE_CONFIG_SCENE, config);
    copy_file(TEMPLATE_CONFIG_A_SCENE, config_all);

    lights = new HueLight *[MAX_HUE_LIGHTS];
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        lights[i] = NULL;
    }
    ConfigJSON::set<const char *>(config, {"name"}, name);
    ConfigJSON::set<const char *>(config_all, {"name"}, name);
}

void HueScene::set_name(const char *n) {
    HueLightGroup::set_name(n);
    ConfigJSON::set<const char *>(config_all, {"name"}, name);
}

void HueScene::set_recycle(const bool r) {
    recycle = r;
    ConfigJSON::set<bool>(config, {"recycle"}, r);
}

void HueScene::set_transition(const uint16_t t) {
    transition = t;
}

HueScene::~HueScene() {
    delete lights;
}