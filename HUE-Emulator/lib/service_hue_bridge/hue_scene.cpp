#include "hue_scene.h"

static const char *TEMPLATE_CONFIG_SCENE = "/hue/s/cf-template.json";
static const char *TEMPLATE_CONFIG_A_SCENE = "/hue/s/cf-a-template.json";

HueScene::HueScene(const char *n, const uint8_t index) :
        HueLightGroup(n, get_file_index_info(HUE_SCENE, index, false),
                      get_file_index_info(HUE_SCENE, index, true),
                      TEMPLATE_CONFIG_SCENE, TEMPLATE_CONFIG_A_SCENE) {
    set_name(n);
}

void HueScene::set_name(const char *n) {
    HueLightGroup::set_name(n);
    ConfigJSON::set<const char *>(cfa->name, {"name"}, n);
    mark_for_reindex();
}

void HueScene::set_recycle(const bool r) {
    ConfigJSON::set<bool>(cf->name, {"recycle"}, r);
    mark_for_reindex();
}