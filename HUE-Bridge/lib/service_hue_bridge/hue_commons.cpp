#include "hue_commons.h"

static FileIndex *info[3][2][MAX_HUE_LIGHTS] = {NULL};

void reindex_all() {
    for (uint8_t t = 0; t < 3; t++) {
        for (uint8_t c = 0; c < 2; c++) {
            for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
                FileIndex *info = get_file_index_info((HueObjectType) t, i, c);
                if (info != NULL && info->refresh) {
                    if (!SPIFFS.exists(info->name)) {
                        info->refresh = false;
                        continue;
                    }
                    File file = SPIFFS.open(info->name, "r");
                    if (!file) continue;
                    info->size = file.size();
                    info->refresh = false;
                    file.close();
                    Log::println("Reindexed %s", info->name);
                }
                yield();
            }
        }
    }
}

FileIndex *get_file_index_info(const HueObjectType t, const uint8_t i, const bool c) {
    if (info[t][c][i] == NULL) {
        info[t][c][i] = new FileIndex;
        info[t][c][i]->name = generate_name(
                (t == HUE_LIGHT ? "/hue/l/" : (t == HUE_GROUP ? "/hue/g/" : "/hue/s/")),
                i, (c ? "-cf-a.json" : "-cf.json"));
        info[t][c][i]->size = 0;
        info[t][c][i]->refresh = true;
    }
    return info[t][c][i];
}

char *generate_name(const char *prefix, uint8_t i, const char *suffix) {
    const uint8_t prefix_len = (uint8_t) strlen(prefix),
            suffix_len = (uint8_t) strlen(suffix),
            total_len = (uint8_t)(prefix_len + suffix_len + 5);
    char index[4] = {'\0'};
    itoa(i, index, 10);

    char *dest = (char *) malloc(sizeof(char) * total_len);
    strcpy(dest, prefix);
    strcpy(dest + prefix_len, index);
    strcpy(dest + prefix_len + strlen(index), suffix);
    return dest;
}

HueObjectConfigStream::HueObjectConfigStream(const HueObjectType t,const bool c) {
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

int HueObjectConfigStream::read(byte *buf, uint16_t nbyte) {
    if (nbyte == 0 || available() == 0) return 0;
    int rd = 0, cp = 0;
    if (p == 0) buf[rd++] = '{';
    for (uint8_t i = 0; i < files_len && rd < nbyte; i++) {
        yield(); // WATCHDOG/WIFI feed
        FileIndex *info = get_file_index_info(type, i, complex);
        if (info->size == 0) continue;
        if (p > cp + info->size) {
            cp += info->size;
        } else {
            String index = (i == 0 ? "\"" : ",\"");
            index += i;
            index += "\":";

            uint32_t rel_pos = p - cp;
            if (rel_pos < index.length()) {
                unsigned int can_read = (unsigned int) nbyte - rd,
                        readed = index.length() > can_read ? can_read : index.length();
                index.getBytes(buf + rd, can_read);
                rd += readed;
                rel_pos -= readed;
            } else {
                rel_pos -= index.length();
            }
            File f = SPIFFS.open(info->name, "r");
            if (!f) continue;

            f.seek(rel_pos, SeekSet);
            rd += f.read(buf + rd, (size_t)(nbyte - rd));
            f.close();
        }
    }
    if (available() - rd == 1) {
        buf[rd++] = '}';
    }
    //while (rd < nbyte) {
    //    buf[rd++] = '\0';
    //}
    p += rd;
    return rd;
}

int HueObjectConfigStream::available() const {
    return (int) ((_size - p) > 0 ? _size - p : 0);
}

uint32_t HueObjectConfigStream::size() const {
    uint32_t *size = (uint32_t * )(&_size);
    *size = 2; // {}
    for (uint8_t i = 0; i < files_len; ++i) {
        FileIndex *info = get_file_index_info(type, i, complex);
        if (info->size == 0) continue;
        *size += (_size == 2 ? 3 : 4); // ,"": /"":
        *size += (i < 10 ? 1 : (i < 100 ? 2 : 3)); // i size
        *size += info->size;
    }
    Log::println("Size %d/", _size);
    return _size;
}

const char *HueObjectConfigStream::name() const { return ""; }

void HueObjectConfigStream::rewind() { p = 0; }

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
    uint16_t to_read = (uint16_t)(templates[i].length() - templates_proceed[i]);
    to_read = to_read < nbyte ? to_read : nbyte;
    if (to_read) {
        for (int o = 0; o < to_read; o++) {
            buf[o] = (byte)templates[i].charAt(o);
        }
    }
    templates_proceed[i] += to_read;
    return to_read;
}

int HueObjectsConfigStream::read(byte *buf, uint16_t nbyte) {
    if (nbyte == 0 || available() == 0) return 0;
    int rd = 0;
    for (uint8_t i = 0; i < stream_len; i++) {
        rd += read(i, buf + rd, (uint16_t)(nbyte - rd));
        rd += streams[i]->read(buf + rd, (uint16_t)(nbyte - rd));
    }
    rd += read(4, buf + rd, (uint16_t)(nbyte - rd));
    p += rd;
    if (rd < nbyte) {
        Log::println("Size mismatch by %d", nbyte - rd);
        Log::println("Size %d/%d", p, size());
    }
    return rd;
}

uint32_t HueObjectsConfigStream::size() const {
    uint32_t *size = (uint32_t * )(&_size);
    *size = 0;
    for (uint8_t i = 0; i < template_len; i++) {
        *size += templates[i].length();
    }
    for (uint8_t i = 0; i < stream_len; i++)
        *size += streams[i]->size();
    Log::println("Size %d", _size);
    return _size;
}

int HueObjectsConfigStream::available() const {
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