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
                }
                yield();
            }
        }
    }
}

void force_reindex() {
    for (uint8_t t = 0; t < 3; t++) {
        for (uint8_t c = 0; c < 2; c++) {
            for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
                FileIndex *info = get_file_index_info((HueObjectType) t, i, c);
                if (info != NULL) {
                    info->refresh = true;
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


void set_string(const char *file_name, const char *param_start, const char *param_end, const char *value) {
    if (file_name == NULL || param_start == NULL || param_end == NULL || value == NULL) return;
    File file = SPIFFS.open(file_name, "r");
    if (!file) return;
    size_t pos_start = 0, pos_end = 0, buff_len = file.size();
    uint8_t buffer[buff_len];
    if (file.find(param_start, strlen(param_start))) {
        pos_start = file.position();
    }
    if (file.find(param_end, strlen(param_end))) {
        pos_end = file.position();
    }
    if (pos_start == 0 || pos_end == 0) {
        file.close();
        return;
    }
    file.seek(0, SeekSet);
    file.read(buffer, file.size());
    file.close();
    file = SPIFFS.open(file_name, "w");
    file.write(buffer, pos_start);
    file.write((uint8_t *) value, strlen(value));
    file.write(buffer + pos_end - strlen(param_end), buff_len - pos_end + strlen(param_end));
    file.flush();
    file.close();
}

HueLight::HueLight(const char *n) {
    set_name(n);
}

void HueLight::set_name(const char *n) {
    if (n == NULL) return;
    checked_free(name);
    name = (char *) malloc(sizeof(char) * (strlen(n) + 1));
    strcpy(name, n);
    if (cf != NULL)
        ConfigJSON::set<const char *>(cf->name, {"name"}, name);
    mark_for_reindex();
}

const char *HueLight::get_name() const {
    return name == NULL ? "" : name;
}

void HueLight::set_color_cie(const float x, const float y) {
    cie_x = x;
    cie_y = y;
    float z = (float) (1.0 - x - y);
    float Y = (float) (get_brightness() / 254.0);
    float X = (Y / y) * x;
    float Z = (Y / y) * z;
    //Convert to RGB using Wide RGB D65 conversion
    float red = (float) (X * 1.656492 - Y * 0.354851 - Z * 0.255038);
    float green = (float) (-X * 0.707196 + Y * 1.655397 + Z * 0.036152);
    float blue = (float) (X * 0.051713 - Y * 0.121364 + Z * 1.011530);
    //If red, green or blue is larger than 1.0 set it back to the maximum of 1.0
    if (red > blue && red > green && red > 1.0) {
        green = green / red;
        blue = blue / red;
        red = 1.0;
    } else if (green > blue && green > red && green > 1.0) {
        red = red / green;
        blue = blue / green;
        green = 1.0;
    } else if (blue > red && blue > green && blue > 1.0) {
        red = red / blue;
        green = green / blue;
        blue = 1.0;
    }
    //Reverse gamma correction
    red = red <= 0.0031308 ? 12.92 * red : (1.0 + 0.055) * pow(red, (1.0 / 2.4)) - 0.055;
    green = green <= 0.0031308 ? 12.92 * green : (1.0 + 0.055) * pow(green, (1.0 / 2.4)) - 0.055;
    blue = blue <= 0.0031308 ? 12.92 * blue : (1.0 + 0.055) * pow(blue, (1.0 / 2.4)) - 0.055;

    set_color_rgb((uint8_t) (red * 255), (uint8_t) (green * 255), (uint8_t) (blue * 255));
}

void HueLight::set_color_ct(const uint32_t ct) {
    float temperature = ct / 100.0f;
    float red = 0, green = 0, blue = 0;
    // Calculate red
    if (temperature <= 66.0) {
        red = 255;
    } else {
        red = temperature - 60.0f;
        red = (float) (329.698727446 * pow((double) red, -0.1332047592));
        if (red < 0) red = 0;
        if (red > 255) red = 255;
    }
    // Calculate green
    if (temperature <= 66.0) {
        green = temperature;
        green = (float) (99.4708025861 * log(green) - 161.1195681661);
        if (green < 0) green = 0;
        if (green > 255) green = 255;
    } else {
        green = temperature - 60.0f;
        green = (float) (288.1221695283 * pow((double) green, -0.0755148492));
        if (green < 0) green = 0;
        if (green > 255) green = 255;
    }
    // Calculate blue
    if (temperature >= 66.0) {
        blue = 255;
    } else {
        if (temperature <= 19.0) {
            blue = 0;
        } else {
            blue = temperature - 10;
            blue = (float) (138.5177312231 * log(blue) - 305.0447927307);
            if (blue < 0) blue = 0;
            if (blue > 255) blue = 255;
        }
    }

    set_color_rgb((uint8_t) red, (uint8_t) green, (uint8_t) blue);
}

void HueLight::set_color_rgb(const uint8_t _r, const uint8_t _g, const uint8_t _b) {
    r = _r;
    g = _g;
    b = _b;
};

HueLight::~HueLight() {
    checked_free(name);
}

uint16_t HueLightGroup::get_hue() const { return hue; }

uint8_t HueLightGroup::get_saturation() const { return sat; }

uint8_t HueLightGroup::get_brightness() const { return bri; }

String HueLightGroup::get_state() const {
    if (state) return "true";
    return "false";
}

void HueLightGroup::for_each_light(LightCallback callback) {
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        if (lights[i] != NULL) {
            callback(lights[i]);
        }
    }
}

bool HueLightGroup::add_light(const uint8_t i, HueLight *l) {
    if (i > MAX_HUE_LIGHTS || l == NULL)
        return false;
    lights[i] = l;
    String index(i);
    ConfigJSON::add_to_array<const char *>(cf->name, {"lights"}, index.c_str());
    ConfigJSON::add_to_array<const char *>(cfa->name, {"lights"}, index.c_str());
    mark_for_reindex();
    return true;
}

void HueLightGroup::clear_lights() {
    for (uint8_t i = 0; i < MAX_HUE_LIGHTS; i++) {
        lights[i] = NULL;
    }
    ConfigJSON::clear_array(cf->name, {"lights"});
    ConfigJSON::clear_array(cfa->name, {"lights"});
    mark_for_reindex();
}

void HueLightGroup::set_color_cie(const float x, const float y) {
    for_each_light([=](HueLight *l) { l->set_color_cie(x, y); });
}

void HueLightGroup::set_color_rgb(const uint8_t r, const uint8_t g, const uint8_t b) {
    for_each_light([=](HueLight *l) { l->set_color_rgb(r, g, b); });
}

void HueLightGroup::set_color_ct(const uint32_t ct) {
    for_each_light([=](HueLight *l) { l->set_color_ct(ct); });
}

void HueLightGroup::set_state(const bool s) {
    state = s;
    for_each_light([=](HueLight *l) { l->set_state(s); });
}

void HueLightGroup::set_hue(const uint16_t h) {
    hue = h;
    for_each_light([=](HueLight *l) { l->set_hue(h); });
}

void HueLightGroup::set_brightness(const uint8_t b) {
    bri = b;
    for_each_light([=](HueLight *l) { l->set_brightness(b); });
}

void HueLightGroup::set_saturation(const uint8_t s) {
    sat = s;
    for_each_light([=](HueLight *l) { l->set_saturation(s); });
}