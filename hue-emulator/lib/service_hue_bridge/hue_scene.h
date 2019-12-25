#ifndef ESP8266_PROJECTS_ROOT_HUE_SCENE_H
#define ESP8266_PROJECTS_ROOT_HUE_SCENE_H

#include <hue_commons.h>

class HueScene : public HueLightGroup {
public:
    HueScene(const char *n, const uint8_t);

    void set_recycle(const bool);

    void set_name(const char *n) override;
};

#endif //ESP8266_PROJECTS_ROOT_HUE_SCENE_H
