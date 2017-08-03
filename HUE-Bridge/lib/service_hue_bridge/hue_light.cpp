#include "hue_light.h"

static const char *TEMPLATE_CONFIG_LIGHT = "/hue/l/cf-template.json";

HueLight::HueLight(const char *n) {
    set_name(n);
}

String HueLight::get_description() const {
    String resp = "{ \"name\": \"";
    resp += get_name();
    return resp + "\"}";
}

String HueLight::get_config() const {
    return "{}";
}

String HueLight::get_config_complex() const { return get_config(); }

void HueLight::set_name(const char *n) {
    if (n == NULL) return;
    checked_free(name);
    name = (char *) malloc(sizeof(char) * (strlen(n) + 1));
    strcpy(name, n);
    ConfigJSON::set<const char *>(config, {"name"}, name);
}

const char *HueLight::get_name() const {
    return name == NULL ? "" : name;
}

void HueLight::set_color_cie(const float x, const float y) {
    cie_x = x;
    cie_y = y;
    float z = 1.0 - x - y;
    float Y = get_brightness() / 254.0;
    float X = (Y / y) * x;
    float Z = (Y / y) * z;

    //Convert to RGB using Wide RGB D65 conversion
    float red = X * 1.656492 - Y * 0.354851 - Z * 0.255038;
    float green = -X * 0.707196 + Y * 1.655397 + Z * 0.036152;
    float blue = X * 0.051713 - Y * 0.121364 + Z * 1.011530;

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

    set_color_rgb((uint8_t)(red * 255), (uint8_t)(green * 255), (uint8_t)(blue * 255));
}

HueLight::~HueLight() {
    checked_free(name);
}

LedLight::LedLight(LedStripService *l, const char *n, const uint8_t index) : HueLight(n) {
    config = get_file_index_info(HUE_LIGHT, index, false)->name;
    config_all = config;
    copy_file(TEMPLATE_CONFIG_LIGHT, config);
    String unique_id = "AA:BB:CC:DD:EE:FF:00:11-";
    unique_id += index;
    ConfigJSON::set<const char *>(config, {"name"}, name);
    ConfigJSON::set<const char *>(config, {"uniqueid"}, unique_id.c_str());
    ls = l;
}

uint16_t LedLight::get_hue() const {
    return ls->get_hue();
};

uint8_t LedLight::get_saturation() const {
    return ls->get_saturation();
};

String LedLight::get_state() const {
    if (ls->get_color())
        return "true";
    return "false";
}

uint8_t LedLight::get_brightness() const {
    return ls->get_brightness();
}

void LedLight::set_color_rgb(const uint8_t r, const uint8_t g, const uint8_t b) {
    ls->set_color(r, g, b);
}

void LedLight::set_state(const bool s) {
    if (s)
        ls->set_color(255, 255, 255);
    else ls->set_color(0, 0, 0);
    ConfigJSON::set<bool>(config, {"state", "on"}, s);
}

void LedLight::set_hue(const uint16_t h) {
    ls->set_hue(h);
    ConfigJSON::set<uint16_t>(config, {"state", "hue"}, h);
}

void LedLight::set_brightness(const uint8_t b) {
    ls->set_brightness(b);
    ConfigJSON::set<uint8_t>(config, {"state", "bri"}, b);
}

void LedLight::set_saturation(const uint8_t s) {
    ls->set_saturation(s);
    ConfigJSON::set<uint8_t>(config, {"state", "sat"}, s);
}