#ifndef ESP8266_PROJECTS_ROOT_HUE_COMMONS_H
#define ESP8266_PROJECTS_ROOT_HUE_COMMONS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <Arduino.h>
#include <file_streams.h>
#include <service_log.h>

#define MAX_HUE_LIGHTS 16
#define MAX_HUE_GROUPS MAX_HUE_LIGHTS
#define MAX_HUE_SCENES MAX_HUE_GROUPS

typedef struct {
    const char *name;
    uint32_t size = 0;
    bool refresh = false;
} FileIndex;

typedef enum {
    HUE_LIGHT, HUE_GROUP, HUE_SCENE, HUE_SENSOR
} HueObjectType;

void reindex_all();

char *generate_name(const char *prefix, uint8_t i, const char *suffix);

FileIndex *get_file_index_info(const HueObjectType t, const uint8_t i, const bool complex = false);

class HueObjectConfigStream : public FStream {
private:
    HueObjectType type;
    uint32_t p = 0, files_len = 0, _size = 0;
    bool complex = false;

public:
    HueObjectConfigStream(const HueObjectType, const bool= false);

    virtual int read(byte *buf, uint16_t nbyte) override;

    int available() const override;

    uint32_t size() const override;

    const char *name() const override;

    void rewind() override;

    ~HueObjectConfigStream();

};

class HueObjectsConfigStream : public FStream {
private:
    static const uint16_t template_len = 5, stream_len = 4;

    String templates[template_len] = {
            "{\"lights\":",
            ",\"groups\":",
            ",\"scenes\":",
            ",\"config\":",
            ",\"schedules\":{},\"sensors\":{},\"rules\":{}}"};
    uint16_t templates_proceed[template_len] = {0};
    FStream *streams[stream_len] = {NULL};
    uint32_t p = 0, _size = 0;
public:
    HueObjectsConfigStream(HueObjectConfigStream *l,
                           HueObjectConfigStream *g,
                           HueObjectConfigStream *s);

    int read(uint8_t i, byte *buf, uint16_t nbyte);

    virtual int read(byte *buf, uint16_t nbyte) override;

    int available() const override;

    uint32_t size() const override;

    const char *name() const override;

    void rewind() override;

    ~HueObjectsConfigStream();

};

#endif //ESP8266_PROJECTS_ROOT_HUE_COMMONS_H
