#ifndef ESP8266_PROJECTS_ROOT_HUE_GROUP_H
#define ESP8266_PROJECTS_ROOT_HUE_GROUP_H

#include <hue_commons.h>

class HueGroup : public HueLightGroup {
public:
    HueGroup(const char *n, const char *t, const uint8_t i);

    void set_color_cie(float x, float y) override;

    void set_color_ct(const uint32_t ct) override;

    void set_state(const bool) override;

    void set_hue(const uint16_t) override;

    void set_brightness(const uint8_t) override;

    void set_saturation(const uint8_t) override;
};

#endif //ESP8266_PROJECTS_ROOT_HUE_GROUP_H
