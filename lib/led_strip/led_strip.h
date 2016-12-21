#ifndef WEMOS_D1_LEDSTRIPS_H_
#define WEMOS_D1_LEDSTRIPS_H_
//#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>
#include <Ticker.h>
#include <esp_api.h>
#include <WiFiUdp.h>

#define STRIP_SERVICE_NAME "_strip_service_"

typedef enum {
    RAINBOW, RAINBOW_CYCLE, REMOTE, OFF = -1
} STRIP_MODES;

RgbColor wheel(byte);

class RGBStrip : public ESP_Service {
public:
    virtual void set_mode(STRIP_MODES);

    virtual void set_brightness(uint8_t);

    virtual uint8_t get_brightness();

    virtual RgbColor get_color();

    virtual void set_color(uint8_t, uint8_t, uint8_t);

    virtual STRIP_MODES get_mode();

    virtual void set_delay(uint32_t);

    virtual uint32_t get_delay();
};

template<typename T_METHOD>
class LedStrip : public RGBStrip {
protected:
    NeoPixelBus<NeoGrbFeature, T_METHOD> *strip = NULL;
    WiFiUDP *udp_socket;
    STRIP_MODES mode = OFF;
    uint8_t strip_brightness = 100;
    uint16_t strip_cycle = 0;
    uint32_t cycle_delay = 0;
    bool timer_running = false;
    RgbColor strip_color;

private:
    void rainbow() {
        for (uint16_t i = 0; i < strip->PixelCount(); i++) {
            strip->SetPixelColor(i, wheel((i + strip_cycle) & 255));
        }
        strip->Show();
        if (strip_cycle++ >= 256)
            strip_cycle = 0;
    }

    void rainbow_cycle() {
        for (uint16_t i = 0; i < strip->PixelCount(); i++) {
            strip->SetPixelColor(
                    i, wheel(((i * 256 / strip->PixelCount()) + strip_cycle) & 255));
        }
        strip->Show();
        if (strip_cycle++ >= 256 * 5)
            strip_cycle = 0;
    }

    [[deprecated]]
    void remote_mode_check() {
        if (mode == REMOTE)
            return;
        if (udp_socket->parsePacket()) {
            unsigned char buffer[5];
            udp_socket->read(buffer, 5);
            if (buffer[0] == 1 && buffer[1] == 1 &&
                buffer[2] == 0 && buffer[3] == 0 && buffer[4] == 0)
                mode = REMOTE;
        }
    }

    void remote() {
        if (udp_socket->parsePacket()) {
            unsigned char buffer[5];
            udp_socket->read(buffer, 5);
            uint16_t address = (uint16_t) ((buffer[0] << 8) | (buffer[1] & 0xff));
            if (!address)
                strip->Show();
            else strip->SetPixelColor(address - 1, RgbColor(buffer[2], buffer[3], buffer[4]));
        }
    }

public:
    LedStrip(const uint16_t led_count, const uint32_t delay = 100) {
        strip = new NeoPixelBus<NeoGrbFeature, T_METHOD>(led_count, 0);
        /*
         * struct espconn;
         * espconn_create();
         */
        udp_socket = new WiFiUDP();
        udp_socket->begin(64500);
        cycle_delay = delay;
        strip->Begin();
    }

    const char *get_name() { return STRIP_SERVICE_NAME; };

    void set_mode(STRIP_MODES new_mode) {
        if (new_mode == OFF) {
            RgbColor color = strip_color;
            set_color(0, 0, 0);
            strip_color = color;
            return;
        }
        mode = new_mode;
    }

    void set_brightness(uint8_t b) {
        if (mode != OFF)
            return;
        strip_brightness = b;
        RgbColor rgb_color((uint8_t) strip_color.R * (b / 100.0),
                           (uint8_t) strip_color.G * (b / 100.0),
                           (uint8_t) strip_color.B * (b / 100.0));
        for (uint16_t i = 0; i < strip->PixelCount(); i++) {
            strip->SetPixelColor(i, rgb_color);
        }
        strip->Show();
    }

    uint8_t get_brightness() { return strip_brightness; }

    RgbColor get_color() { return strip_color; }

    void set_color(uint8_t r, uint8_t g, uint8_t b) {
        mode = OFF;
        strip_color = RgbColor(r, g, b);
        set_brightness(strip_brightness);
    }

    STRIP_MODES get_mode() { return mode; }

    void cycle_routine() {
        remote_mode_check();
        switch (mode) {
            case OFF:
                return;
            case REMOTE:
                remote();
                return;
            case RAINBOW:
                rainbow();
                break;
            case RAINBOW_CYCLE:
                rainbow_cycle();
                break;
        }
        delay(cycle_delay);
    }

    void set_delay(uint32_t delay) { cycle_delay = delay; }

    uint32_t get_delay() { return cycle_delay; }

    virtual ~LedStrip() {
        delete strip;
        delete udp_socket;
    }
};

#endif /* WEMOS_D1_LEDSTRIPS_H_ */
