#ifndef ESP8266_PROJECTS_ROOT_HUE_COMMONS_H
#define ESP8266_PROJECTS_ROOT_HUE_COMMONS_H

#include <file_system.h>

#define MAX_HUE_LIGHTS 16
#define MAX_HUE_GROUPS MAX_HUE_LIGHTS
#define MAX_HUE_SCENES MAX_HUE_GROUPS

typedef struct {
    const char *name;
    uint32_t size = 0;
    bool refresh = false;
} FileIndex;

typedef enum {
    HUE_LIGHT, HUE_GROUP, HUE_SCENE
} HueObjectType;

class ConfigObject {
protected:
    FileIndex *cf = NULL, *cfa = NULL;
public:
    void mark_for_reindex() {
        if (cf != NULL) {
            cf->refresh = true;
        }
        if (cfa != NULL) {
            cfa->refresh = true;
        }
    }
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

    virtual void set_color_cie(const float x, const float y);

    virtual void set_hue(const uint16_t)=0;

    virtual void set_state(const bool)=0;

    virtual void set_color_rgb(const uint8_t, const uint8_t, const uint8_t)=0;

    virtual void set_brightness(const uint8_t)=0;

    virtual void set_saturation(const uint8_t)=0;

    virtual ~HueLight();
};

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

    virtual void set_color_cie(const float x, const float y);

    void set_color_rgb(const uint8_t, const uint8_t, const uint8_t);

    void set_state(const bool) override;

    void set_hue(const uint16_t) override;

    void set_brightness(const uint8_t) override;

    void set_saturation(const uint8_t) override;
};

void reindex_all();
void force_reindex();

char *generate_name(const char *prefix, uint8_t i, const char *suffix);

FileIndex *get_file_index_info(const HueObjectType t, const uint8_t i, const bool complex = false);

void set_string(const char *file_name, const char *param_start, const char *param_end, const char *value);

#endif //ESP8266_PROJECTS_ROOT_HUE_COMMONS_H
