#ifndef ESP8266_PROJECTS_ROOT_HUE_COMMONS_H
#define ESP8266_PROJECTS_ROOT_HUE_COMMONS_H

#include <file_system.h>
#include <ESP8266HTTPClient.h>

#define MAX_HUE_LIGHTS 16
#define MAX_HUE_GROUPS MAX_HUE_LIGHTS
#define MAX_HUE_SCENES MAX_HUE_GROUPS

typedef struct {
    const char *name = NULL;
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
    ConfigObject(FileIndex *_cf, FileIndex *_cfa, const char *f_template, const char *f_template_all) {
        cf = _cf;
        cfa = _cfa;
        copy_file(f_template, cf->name);
        copy_file(f_template_all, cfa->name);
    }

    void mark_for_reindex() {
        if (cf != NULL) {
            cf->refresh = true;
        }
        if (cfa != NULL) {
            cfa->refresh = true;
        }
    }

    virtual ~ConfigObject() {
        if (cf != NULL && cf->size != 0) {
            SPIFFS.remove(cf->name);
            cf->size = 0;
        }
        if (cfa != NULL && cfa->size != 0) {
            SPIFFS.remove(cfa->name);
            cfa->size = 0;
        }
    }
};

class HueLight : public ConfigObject {
protected:
    uint8_t r = 0, g = 0, b = 0;

    virtual uint8_t get_brightness() const { return 255; };
public:
    HueLight(const char *n, FileIndex *f_info, const char *f_template);

    HueLight(const char *n, FileIndex *f_info, FileIndex *f_info_all, const char *f_template,
             const char *f_template_all);

    virtual void set_color_cie(const float x, const float y);

    virtual void set_color_ct(const uint32_t ct);

    virtual void set_color_rgb(const uint8_t, const uint8_t, const uint8_t);

    virtual void set_name(const char *n);

    virtual void set_hue(const uint16_t)=0;

    virtual void set_state(const bool)=0;

    virtual void set_brightness(const uint8_t)=0;

    virtual void set_saturation(const uint8_t)=0;

    virtual void set_transition(const uint16_t)=0;
};

class HueLightGroup : public HueLight {
protected:
    HueLight **bridge_lights = NULL;
    bool lights[MAX_HUE_LIGHTS] = {false};

    typedef std::function<void(HueLight *)> LightCallback;

    void for_each_light(LightCallback);

public:
    HueLightGroup(const char *n, FileIndex *f_info, FileIndex *f_info_all, const char *f_template,
                  const char *f_template_all);

    void set_bridge_lights(HueLight **all_lights);

    virtual bool add_light(const uint8_t);

    virtual void clear_lights();

    virtual void set_color_cie(const float x, const float y);

    virtual void set_color_rgb(const uint8_t, const uint8_t, const uint8_t);

    virtual void set_color_ct(const uint32_t ct);

    virtual void set_state(const bool) override;

    virtual void set_hue(const uint16_t) override;

    virtual void set_brightness(const uint8_t) override;

    virtual void set_saturation(const uint8_t) override;

    virtual void set_transition(const uint16_t);
};

void reindex_all();

void resend_queries();
static String sendPOST(IPAddress ipAddress, String uri, String data);

FileIndex *get_file_index_info(const HueObjectType t, const uint8_t i, const bool complex = false);

void set_string(const char *file_name, const char *param_start, const char *param_end, const char *value);

#endif //ESP8266_PROJECTS_ROOT_HUE_COMMONS_H
