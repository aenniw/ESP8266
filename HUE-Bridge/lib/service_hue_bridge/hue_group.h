#ifndef ESP8266_PROJECTS_ROOT_HUE_GROUP_H
#define ESP8266_PROJECTS_ROOT_HUE_GROUP_H

#include <hue_commons.h>

class HueGroup : public HueLightGroup {
private:
    char *type = NULL;
protected:
    void set_type(const char *);

public:
    HueGroup(HueLight **);

    HueGroup(const char *n, const char *t, const uint8_t i);

    bool add_light(const uint8_t, HueLight *);

    void clear_lights();

    void set_color_cie(float x, float y) override;

    void set_state(const bool) override;

    void set_hue(const uint16_t) override;

    void set_brightness(const uint8_t) override;

    void set_saturation(const uint8_t) override;

    ~HueGroup();
};

#endif //ESP8266_PROJECTS_ROOT_HUE_GROUP_H
