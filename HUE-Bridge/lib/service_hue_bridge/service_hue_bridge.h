#ifndef ESP8266_PROJECTS_ROOT_SERVICE_HUE_BRIDGE_H
#define ESP8266_PROJECTS_ROOT_SERVICE_HUE_BRIDGE_H

#include <SSDP.h>
#include <commons.h>
#include <commons_json.h>
#include <service_rest.h>
#include <hue_light.h>
#include <hue_group.h>
#include <hue_scene.h>
#include <hue_config_streams.h>

class HueBridge : public Service {
private:
    HueLight *lights[MAX_HUE_LIGHTS] = {NULL};
    HueGroup *groups[MAX_HUE_GROUPS] = {NULL};
    HueScene *scenes[MAX_HUE_SCENES] = {NULL};
    String bridgeIDString;

    void initialize_SSDP();

    void initialize_config(RestService *web_service);

    void initialize_lights(RestService *web_service);

    void initialize_groups(RestService *web_service);

    void initialize_scenes(RestService *web_service);

    void restore_groups();

    void restore_scenes();

protected:
    HueLight *get_light(const uint8_t i) const;

    HueGroup *get_group(const uint8_t i) const;

    HueScene *get_scene(const uint8_t i) const;

    int16_t get_free_index(ConfigObject *const *ls, const uint8_t len, const uint8_t s = 0) const;

    String update_hue_lights(const String &arg, const String &uri, const String &path, HueLight *lights[],
                             const uint16_t l);

    String update_hue_groups(const String &arg, const String &uri, const String &path, HueLightGroup *lightGroups[],
                             const uint16_t l);
public:
    HueBridge(RestService *web_service);

    int8_t add_light(LedStripService *);

    int8_t add_group(const char *n, const char *t);

    int8_t add_scene(const char *n);

    bool delete_light(const uint8_t);

    bool delete_group(const uint8_t);

    bool delete_scene(const uint8_t);

    void cycle_routine();

    ~HueBridge();
};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_HUE_BRIDGE_H
