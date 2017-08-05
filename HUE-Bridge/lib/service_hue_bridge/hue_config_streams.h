#ifndef ESP8266_PROJECTS_ROOT_CONFIG_STREAMS_H
#define ESP8266_PROJECTS_ROOT_CONFIG_STREAMS_H

#include <file_streams.h>
#include <hue_commons.h>

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


#endif //ESP8266_PROJECTS_ROOT_CONFIG_STREAMS_H
