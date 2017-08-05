#ifndef ESP8266_PROJECTS_ROOT_HUE_LIGHT_H
#define ESP8266_PROJECTS_ROOT_HUE_LIGHT_H

#include <hue_commons.h>
#include <service_led_strip.h>

class LedLight : public HueLight {
private:
    LedStripService *ls;
protected:
    uint16_t get_hue() const override;

    uint8_t get_saturation() const override;

    String get_state() const override;

    uint8_t get_brightness() const override;

public:
    LedLight(LedStripService *l, const char *n, const uint8_t index);

    void set_color_cie(float x, float y) override;

    void set_color_rgb(const uint8_t r, const uint8_t g, const uint8_t b);

    void set_state(const bool s) override;

    void set_hue(const uint16_t h) override;

    void set_brightness(const uint8_t b) override;

    void set_saturation(const uint8_t s) override;

    ~LedLight() {};
};

#endif //ESP8266_PROJECTS_ROOT_HUE_LIGHT_H
