#include "hue_config_streams.h"

HueObjectConfigStream::HueObjectConfigStream(const HueObjectType t, const bool c) {
    type = t;
    switch (type) {
        case HUE_LIGHT:
            files_len = MAX_HUE_LIGHTS;
            break;
        case HUE_GROUP:
            files_len = MAX_HUE_GROUPS;
            break;
        case HUE_SCENE:
            files_len = MAX_HUE_SCENES;
            break;
    }
    complex = c;
}

int HueObjectConfigStream::read(uint8_t i, byte *buf, uint16_t nbyte) {
    if (nbyte == 0) return 0;
    String index = (i == 0 ? "\"" : ",\"");
    index += i;
    index += "\":";
    uint16_t to_read = (uint16_t)(index.length() - templates_proceed[i]);
    to_read = to_read < nbyte ? to_read : nbyte;
    for (int o = 0; o < to_read; o++) {
        buf[o] = (byte) index.charAt(o);
    }
    templates_proceed[i] += to_read;
    return to_read;
}

int HueObjectConfigStream::read_f(uint8_t i, byte *buf, uint16_t nbyte) {
    if (nbyte == 0) return 0;
    FileIndex *info = get_file_index_info(type, i, complex);
    int rd = 0;
    File f = SPIFFS.open(info->name, "r");
    if (!f) return 0;
    f.seek(files_proceed[i], SeekSet);
    rd = f.read(buf, nbyte);
    f.close();
    files_proceed[i] += rd;
    return rd;
}

int HueObjectConfigStream::read(byte *buf, uint16_t nbyte) {
    if (nbyte == 0 || available() == 0) return 0;
    int rd = 0;
    if (p == 0) buf[rd++] = '{';
    for (uint8_t i = 0; i < files_len && rd < nbyte; i++) {
        yield(); // WATCHDOG/WIFI feed
        FileIndex *info = get_file_index_info(type, i, complex);
        if (info->size == 0) continue;
        rd += read(i, buf + rd, (uint16_t)(nbyte - rd));
        rd += read_f(i, buf + rd, (uint16_t)(nbyte - rd));
    }
    if (available() - rd == 1) {
        buf[rd++] = '}';
    }
    p += rd;
    return rd;
}

int HueObjectConfigStream::available() {
    return (int) ((_size - p) > 0 ? _size - p : 0);
}

uint32_t HueObjectConfigStream::size() const {
    uint32_t *size = (uint32_t *) (&_size);
    *size = 2; // {}
    for (uint8_t i = 0; i < files_len; ++i) {
        FileIndex *info = get_file_index_info(type, i, complex);
        if (info->size == 0) continue;
        *size += (_size == 2 ? 3 : 4); // ,"": /"":
        *size += (i < 10 ? 1 : (i < 100 ? 2 : 3)); // i size
        *size += info->size;
    }
    return _size;
}

const char *HueObjectConfigStream::name() const { return ""; }

void HueObjectConfigStream::rewind() {
    p = 0;
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++)
        templates_proceed[i] = 0;
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++)
        files_proceed[i] = 0;
}

HueObjectConfigStream::~HueObjectConfigStream() {}

HueObjectsConfigStream::HueObjectsConfigStream(HueObjectConfigStream *l, HueObjectConfigStream *g,
                                               HueObjectConfigStream *s) {
    streams[HUE_LIGHT] = l;
    streams[HUE_GROUP] = g;
    streams[HUE_SCENE] = s;
    streams[3] = new MultiFileStream({"/hue/config.json"});
}

int HueObjectsConfigStream::read(uint8_t i, byte *buf, uint16_t nbyte) {
    if (nbyte == 0) return 0;
    uint16_t to_read = (uint16_t) (templates[i].length() - templates_proceed[i]);
    to_read = to_read < nbyte ? to_read : nbyte;
    for (int o = 0; o < to_read; o++) {
        buf[o] = (byte) templates[i].charAt(o);
    }
    templates_proceed[i] += to_read;
    return to_read;
}

int HueObjectsConfigStream::read(byte *buf, uint16_t nbyte) {
    if (nbyte == 0 || available() == 0) return 0;
    int rd = 0;
    for (uint8_t i = 0; i < stream_len; i++) {
        rd += read(i, buf + rd, (uint16_t) (nbyte - rd));
        rd += streams[i]->read(buf + rd, (uint16_t) (nbyte - rd));
    }
    rd += read(4, buf + rd, (uint16_t) (nbyte - rd));
    p += rd;
    return rd;
}

uint32_t HueObjectsConfigStream::size() const {
    uint32_t *size = (uint32_t *) (&_size);
    *size = 0;
    for (uint8_t i = 0; i < template_len; i++) {
        *size += templates[i].length();
    }
    for (uint8_t i = 0; i < stream_len; i++)
        *size += streams[i]->size();
    return _size;
}

int HueObjectsConfigStream::available() {
    return (int) ((_size - p) > 0 ? _size - p : 0);
}

const char *HueObjectsConfigStream::name() const { return ""; }

void HueObjectsConfigStream::rewind() {
    p = 0;
    for (uint8_t i = 0; i < stream_len; i++)
        streams[i]->rewind();
    for (uint8_t i = 0; i < template_len; i++)
        templates_proceed[i] = 0;
}

HueObjectsConfigStream::~HueObjectsConfigStream() {}
