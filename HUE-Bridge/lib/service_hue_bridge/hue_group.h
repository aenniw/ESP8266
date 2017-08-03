#ifndef ESP8266_PROJECTS_ROOT_HUE_GROUP_H
#define ESP8266_PROJECTS_ROOT_HUE_GROUP_H

#include <hue_light.h>

class HueLightGroup : public HueLight {
protected:
    bool state = false;
    uint16_t hue = 0, ct = 500;
    uint8_t sat = 0, bri = 0;
    HueLight **lights = NULL;

    typedef std::function<void(HueLight *)> LightCallback;

    uint16_t get_hue() const override;

    uint8_t get_saturation() const override;

    uint8_t get_brightness() const override;

    String get_state() const override;

    void for_each_light(LightCallback);

public:
    HueLightGroup(const char *n) : HueLight(n) {}

    virtual bool add_light(const uint8_t, HueLight *);

    virtual void clear_lights();

    void set_color_rgb(const uint8_t, const uint8_t, const uint8_t);

    void set_state(const bool) override;

    void set_hue(const uint16_t) override;

    void set_brightness(const uint8_t) override;

    void set_saturation(const uint8_t) override;
};

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

class HueGroup : public HueLightGroup {
private:
    bool global = false;
    char *type = NULL;
protected:
    void set_type(const char *);

public:
    HueGroup(HueLight **);

    HueGroup(const char *n, const char *t, const uint8_t i);

    bool add_light(const uint8_t, HueLight *);

    void clear_lights();

    String get_config() const override;

    String get_config_complex() const override;

    void set_state(const bool) override;

    void set_hue(const uint16_t) override;

    void set_brightness(const uint8_t) override;

    void set_saturation(const uint8_t) override;

    ~HueGroup();
};

#endif //ESP8266_PROJECTS_ROOT_HUE_GROUP_H
