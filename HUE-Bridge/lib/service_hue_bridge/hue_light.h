#ifndef ESP8266_PROJECTS_ROOT_HUE_LIGHT_H
#define ESP8266_PROJECTS_ROOT_HUE_LIGHT_H

#include <file_system.h>
#include <service_led_strip.h>
#include <hue_commons.h>

#define HUE_LIGHT_MODEL "LST001"

class ConfigObject {
protected:
    const char *config = NULL, *config_all = NULL;
public:
    virtual String get_config() const = 0;

    virtual String get_description() const = 0;

    virtual String get_config_complex() const = 0;
};

class HueLight : public ConfigObject {
protected:
    char *name = NULL;
    float cie_x = 0.5, cie_y = 0.5;

    virtual uint16_t get_hue() const = 0;
    virtual uint8_t get_saturation() const =0;
    virtual String get_state() const = 0;
    virtual uint8_t get_brightness() const =0;
public:
    HueLight(const char *n = "Hue Light");

    virtual void set_name(const char *n);
    const char *get_name() const;

    void set_color_cie(const float x, const float y);

    virtual String get_config() const override;

    virtual String get_description() const override;

    virtual String get_config_complex() const override;

    virtual void set_hue(const uint16_t)=0;
    virtual void set_state(const bool)=0;
    virtual void set_color_rgb(const uint8_t, const uint8_t, const uint8_t)=0;
    virtual void set_brightness(const uint8_t)=0;
    virtual void set_saturation(const uint8_t)=0;

    virtual ~HueLight();
};

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

    void set_color_rgb(const uint8_t r, const uint8_t g, const uint8_t b);

    void set_state(const bool s) override;

    void set_hue(const uint16_t h) override;

    void set_brightness(const uint8_t b) override;

    void set_saturation(const uint8_t s) override;

    ~LedLight() {};
};

#endif //ESP8266_PROJECTS_ROOT_HUE_LIGHT_H
