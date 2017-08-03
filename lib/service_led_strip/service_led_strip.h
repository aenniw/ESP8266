#ifndef ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_H
#define ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_H

#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>
#include <commons.h>

class NeoPixelBusInterface {
public:
    virtual void set_all_pixels(const HsbColor) = 0;

    virtual void set_pixel(const uint16_t, const HsbColor) = 0;

    virtual void refresh() = 0;

    virtual uint16_t get_len() const = 0;
};

template<typename TYPE, typename MODE>
class PixelBus : public NeoPixelBus<TYPE, MODE>, public NeoPixelBusInterface {
public:
    PixelBus(uint16_t countPixels) : NeoPixelBus<TYPE, MODE>(countPixels, 0) {
        NeoPixelBus<TYPE, MODE>::Begin();
    }

    void set_all_pixels(const HsbColor color) {
        NeoPixelBus<TYPE, MODE>::ClearTo(color);
    }

    void set_pixel(const uint16_t i, const HsbColor color) {
        NeoPixelBus<TYPE, MODE>::SetPixelColor(i, color);
    }

    void refresh() {
        if (NeoPixelBus<TYPE, MODE>::CanShow())
            NeoPixelBus<TYPE, MODE>::Show();
    }

    uint16_t get_len() const {
        return NeoPixelBus<TYPE, MODE>::PixelCount();
    }
};

typedef enum {
    SINGLE_COLOR, ANIMATION_0, ANIMATION_1
} LED_STRIP_ANIM_MODE;

typedef enum {
    GRB, GRBW
} LED_STRIP_TYPE;

typedef enum {
    DMA800, UART800
} LED_STRIP_TRANSFER_MODE;

class LedStripService : public Service {
private:
    HsbColor color = HsbColor(0, 0, 0);
    LED_STRIP_ANIM_MODE mode = SINGLE_COLOR;
    LED_STRIP_TYPE type;
    LED_STRIP_TRANSFER_MODE t_mode;
    NeoPixelBusInterface *led_strip = NULL;
    NeoPixelAnimator *animator = NULL;
protected:
    void animation_0(const AnimationParam &param);

    void animation_1(const AnimationParam &param);

    virtual void set_color(const uint32_t);

public:
    LedStripService(const LED_STRIP_TYPE, const LED_STRIP_TRANSFER_MODE, const uint16_t len);

    LED_STRIP_TYPE get_type() const { return type; }

    LED_STRIP_TRANSFER_MODE get_transfer_mode() const { return t_mode; }

    uint16_t get_len() const { return led_strip->get_len(); }

    void set_color(const uint8_t, const uint8_t, const uint8_t);

    uint32_t get_color() const;

    void set_hue(const uint16_t h);

    void set_saturation(const uint8_t s);

    virtual void set_brightness(const uint8_t);

    uint16_t get_hue() const;

    uint8_t get_saturation() const;

    uint8_t get_brightness() const;

    void cycle_routine();

    void set_delay(const uint16_t);

    uint16_t get_delay() const { return animator->getTimeScale(); }

    virtual void set_mode(const LED_STRIP_ANIM_MODE);

    LED_STRIP_ANIM_MODE get_mode() const { return mode; }

    ~LedStripService();
};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_H
