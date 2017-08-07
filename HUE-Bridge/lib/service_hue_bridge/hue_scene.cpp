#include "hue_scene.h"

static const char *TEMPLATE_CONFIG_SCENE = "/hue/s/cf-template.json";
static const char *TEMPLATE_CONFIG_A_SCENE = "/hue/s/cf-a-template.json";

HueScene::HueScene(const char *n, const uint8_t index) : HueLightGroup(n) {
    cf = get_file_index_info(HUE_SCENE, index, false),
            cfa = get_file_index_info(HUE_SCENE, index, true);

    copy_file(TEMPLATE_CONFIG_SCENE, cf->name);
    copy_file(TEMPLATE_CONFIG_A_SCENE, cfa->name);

    lights = new HueLight *[MAX_HUE_LIGHTS];
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        lights[i] = NULL;
    }
    ConfigJSON::set<const char *>(cf->name, {"name"}, name);
    ConfigJSON::set<const char *>(cfa->name, {"name"}, name);
    mark_for_reindex();
}

void HueScene::set_name(const char *n) {
    ConfigJSON::set<const char *>(cfa->name, {"name"}, name);
    HueLightGroup::set_name(n);
}

void HueScene::set_recycle(const bool r) {
    recycle = r;
    ConfigJSON::set<bool>(cf->name, {"recycle"}, r);
    mark_for_reindex();
}

void HueScene::set_transition(const uint16_t t) {
    transition = t;
}

HueScene::~HueScene() {
    delete lights;
}