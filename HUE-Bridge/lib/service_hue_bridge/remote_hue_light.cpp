#include "remote_hue_light.h"

static const char *TEMPLATE_CONFIG_LIGHT = "/hue/l/cf-template.json";

static void sendHSB(uint16_t h, uint8_t s, uint8_t b, IPAddress &ipAddress) {
    String payload("{ \"hsb\":");
    payload += (uint32_t) (((0x0000FFFF & h) << 16) | ((0x000000FF & s) << 8) | (b));
    payload += "}";
    sendPOST(ipAddress, "/led-strip/set-color", payload);
}

RemoteLedLight::RemoteLedLight(const IPAddress &address, const char *n, const uint8_t index)
        : HueLight(n, get_file_index_info(HUE_LIGHT, index, false), TEMPLATE_CONFIG_LIGHT) {
    ipAddress = IPAddress(address);
    String unique_id = "AA:BB:CC:DD:EE:FF:00:11-";
    unique_id += index;
    ConfigJSON::set<const char *>(cf->name, {"uniqueid"}, unique_id.c_str());
    ConfigJSON::set<bool>(cf->name, {"state", "on"}, false);
    mark_for_reindex();
}

void RemoteLedLight::set_color_cie(float x, float y) {
    HueLight::set_color_cie(x, y);
    ConfigJSON::clear_array(cf->name, {"state", "xy"});
    ConfigJSON::add_to_array<float>(cf->name, {"state", "xy"}, x);
    ConfigJSON::add_to_array<float>(cf->name, {"state", "xy"}, y);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "xy");
    mark_for_reindex();
}

void RemoteLedLight::set_color_ct(const uint32_t ct) {
    HueLight::set_color_ct(ct);
    ConfigJSON::set<uint32_t>(cf->name, {"state", "ct"}, ct);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "ct");
    mark_for_reindex();
}

void RemoteLedLight::set_color_rgb(const uint8_t _r, const uint8_t _g, const uint8_t _b) {
    String payload("{ \"rgb\":");
    payload += (uint32_t) (((0x000000FF & _r) << 16) | ((0x000000FF & _g) << 8) | (_b));
    payload += "}";
    sendPOST(ipAddress, "/led-strip/set-color", payload);
    HueLight::set_color_rgb(_r, _g, _b);
}

void RemoteLedLight::set_state(const bool s) {
    if (s) {
        String payload("{ \"rgb\":");
        payload += (uint32_t) (((0x000000FF & r) << 16) | ((0x000000FF & g) << 8) | (b));
        payload += "}";
        sendPOST(ipAddress, "/led-strip/set-color", payload);
    } else {
        String payload("{ \"rgb\":0 }");
        sendPOST(ipAddress, "/led-strip/set-color", payload);
    }
    ConfigJSON::set<bool>(cf->name, {"state", "on"}, s);
    mark_for_reindex();
}

void RemoteLedLight::set_hue(const uint16_t h) {
    hue = h;
    sendHSB(hue, sat, bri, ipAddress);
    ConfigJSON::set<uint16_t>(cf->name, {"state", "hue"}, h);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "hs");
    mark_for_reindex();
}

void RemoteLedLight::set_brightness(const uint8_t b) {
    bri = b;
    sendHSB(hue, sat, bri, ipAddress);
    ConfigJSON::set<uint8_t>(cf->name, {"state", "bri"}, b);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "hs");
    mark_for_reindex();
}

void RemoteLedLight::set_saturation(const uint8_t s) {
    sat = s;
    sendHSB(hue, sat, bri, ipAddress);
    ConfigJSON::set<uint8_t>(cf->name, {"state", "sat"}, s);
    ConfigJSON::set<const char *>(cf->name, {"state", "colormode"}, "hs");
    mark_for_reindex();
}

void RemoteLedLight::set_transition(const uint16_t t) {
    //ls->set_delay(uint16_t(t * 0.25f));
}