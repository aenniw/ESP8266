#ifndef WEMOS_D1_LEDSTRIPS_H_
#define WEMOS_D1_LEDSTRIPS_H_
//#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>
#include <Ticker.h>

typedef enum {
    RAINBOW, RAINBOW_CYCLE, OFF = -1
} STRIP_MODES;

RgbColor wheel(byte);

template<typename T_METHOD>
class LedStrip {
    NeoPixelBus<NeoGrbFeature, T_METHOD> *strip = NULL;
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

public:
    LedStrip(const uint8_t led_count, const uint32_t delay = 100) {
        strip = new NeoPixelBus<NeoGrbFeature, T_METHOD>(led_count, 0);
        cycle_delay = delay;
        strip->Begin();
    }

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
        switch (mode) {
            case OFF:
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

    virtual ~LedStrip() { delete strip; }
};

#endif /* WEMOS_D1_LEDSTRIPS_H_ */
