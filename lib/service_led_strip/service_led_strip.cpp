#include "service_led_strip.h"

#define ANIMATION_INDEX 0
#define ANIMATION_END 1000

static RgbColor wheel(uint8_t pos) {
    if (pos < 85) {
        return RgbColor(pos * 3, 255 - pos * 3, 0);
    } else if (pos < 170) {
        pos -= 85;
        return RgbColor(255 - pos * 3, 0, pos * 3);
    } else {
        pos -= 170;
        return RgbColor(0, pos * 3, 255 - pos * 3);
    }
}

// TODO: use something more optimized
static HsbColor wheel(const uint8_t pos, const float b) {
    HsbColor color = HsbColor(wheel(pos));
    color.B = b;
    return color;
}

void LedStripService::animation_0(const AnimationParam &param) {
    const uint8_t step = (uint8_t)(param.progress * 255);
    for (uint16_t i = 0; i < get_len(); i++) {
        led_strip->set_pixel(i, wheel((uint8_t)(i + step), color.B));
    }
    if (param.state == AnimationState_Completed) {
        animator->RestartAnimation(ANIMATION_INDEX);
    }
}

void LedStripService::animation_1(const AnimationParam &param) {
    const uint8_t step = (uint8_t)(param.progress * 255);
    for (uint16_t i = 0; i < get_len(); i++) {
        led_strip->set_pixel(i, wheel((uint8_t)((i * 256 / get_len()) + step), color.B));
    }
    if (param.state == AnimationState_Completed) {
        animator->RestartAnimation(ANIMATION_INDEX);
    }
}

void LedStripService::animation_transition(const AnimationParam &param, HsbColor old_color) {
    led_strip->set_all_pixels(RgbColor::LinearBlend(old_color, color, param.progress));
}

LedStripService::LedStripService(const LED_STRIP_TYPE t, const LED_STRIP_TRANSFER_MODE mode, const uint16_t len) {
    animator = new NeoPixelAnimator(1, 10);
    t_mode = mode;
    type = t;
    switch (type) {
        case GRB:
            switch (t_mode) {
                case DMA800:
                    led_strip = new PixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod>(len);
                    return;
                case UART800:
                    led_strip = new PixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>(len);
                    return;
            }
        case GRBW:
            switch (t_mode) {
                case DMA800:
                    led_strip = new PixelBus<NeoGrbwFeature, NeoEsp8266Dma800KbpsMethod>(len);
                    return;
                case UART800:
                    led_strip = new PixelBus<NeoGrbwFeature, NeoEsp8266Uart800KbpsMethod>(len);
                    return;
            }
    }
    set_rgb(0, 0, 0);
}

LedStripService::~LedStripService() {
    delete animator;
}

//TODO rename to set_rgb
void LedStripService::set_color(const uint32_t color) {
    set_rgb((uint8_t) ((0x00FF0000 & color) >> 16), (uint8_t) ((0x0000FF00 & color) >> 8),
            (uint8_t)(0x000000FF & color));
}

void LedStripService::set_animated_color_change(const bool c) {
    animated_color_change = c;
}

//TODO rename to set_hsb
void LedStripService::set_rgb(const uint8_t r, const uint8_t g, const uint8_t b) {
    if (SINGLE_COLOR != mode) {
        animator->StopAll();
        mode = SINGLE_COLOR;
    }
    if (animated_color_change) {
        HsbColor old_color = color;
        color = HsbColor(RgbColor(r, g, b));
        animator->StartAnimation(ANIMATION_INDEX, ANIMATION_END,
                                 [old_color, this](const AnimationParam &param) {
                                     animation_transition(param, old_color);
                                 });
    } else {
        color = HsbColor(RgbColor(r, g, b));
        led_strip->set_all_pixels(color);
    }
}

uint32_t LedStripService::get_rgb() const {
    const RgbColor rgb = RgbColor(color);
    return (uint32_t)((0x00FF0000 & rgb.R << 16) | (0x0000FF00 & rgb.G << 8) | rgb.B);
}

void LedStripService::set_hsb(uint32_t c) {
    const uint16_t h = (uint16_t) ((0xFFFF0000 & c) >> 16);
    const uint8_t s = (uint8_t) ((0x0000FF00 & c) >> 8),
            b = (uint8_t) (0x000000FF & c);
    color = HsbColor((h % 65535) / (float) 100, (s % 101) / (float) 100, (b % 101) / (float) 100);
    led_strip->set_all_pixels(color);
}

void LedStripService::set_hue(const uint16_t h) {
    color = HsbColor((h % 65535) / (float) 100, color.S, color.B);
    led_strip->set_all_pixels(color);
}

void LedStripService::set_saturation(const uint8_t s) {
    color = HsbColor(color.H, (s % 101) / (float) 100, color.B);
    led_strip->set_all_pixels(color);
}

void LedStripService::set_brightness(const uint8_t b) {
    color = HsbColor(color.H, color.S, (b % 101) / (float) 100);
    led_strip->set_all_pixels(color);
}

uint16_t LedStripService::get_hue() const {
    return (uint8_t) (color.H * 100);
}

uint8_t LedStripService::get_saturation() const {
    return (uint8_t) (color.S * 100);
}

uint8_t LedStripService::get_brightness() const {
    return (uint8_t) (color.B * 100);
}

void LedStripService::set_delay(const uint16_t d) {
    animator->setTimeScale(d);
}

void LedStripService::set_mode(const LED_STRIP_ANIM_MODE m) {
    animator->StopAll();
    switch ((mode = m)) {
        case SINGLE_COLOR:
            led_strip->set_all_pixels(color);
            return;
        case ANIMATION_0:
            animator->StartAnimation(ANIMATION_INDEX, ANIMATION_END, [this](const AnimationParam &param) {
                animation_0(param);
            });
            break;
        case ANIMATION_1:
            animator->StartAnimation(ANIMATION_INDEX, ANIMATION_END, [this](const AnimationParam &param) {
                animation_1(param);
            });
            break;
    }
}

void LedStripService::cycle_routine() {
    // Call at least every 10ms
    if (animator != NULL && animator->IsAnimating())
        animator->UpdateAnimations();
    led_strip->refresh();
}
