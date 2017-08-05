#ifndef ESP8266_PROJECTS_ROOT_HUE_SCENE_H
#define ESP8266_PROJECTS_ROOT_HUE_SCENE_H

#include <hue_commons.h>

class HueScene : public HueLightGroup {
private:
    bool recycle = false;
    uint16_t transition = 0;
public:
    HueScene(const char *n, const uint8_t);

    void set_recycle(const bool);

    void set_transition(const uint16_t);

    void set_name(const char *n) override;

    ~HueScene();
};

#endif //ESP8266_PROJECTS_ROOT_HUE_SCENE_H
