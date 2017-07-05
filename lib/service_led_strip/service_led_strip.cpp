#include "service_led_strip.h"

#define ANIMATION_BEGIN 0
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

void animation_(const AnimationParam &param) {

}

void LedStripService::animation_0(const AnimationParam &param) {
    for (uint16_t i = 0; i < led_strip->PixelCount(); i++) {
        led_strip->SetPixelColor(i, wheel((i + (param.index % 256)) & 255));
    }
}

void LedStripService::animation_1(const AnimationParam &param) {
    for (uint16_t i = 0; i < led_strip->PixelCount(); i++) {
        led_strip->SetPixelColor(
                i, wheel(((i * 256 / led_strip->PixelCount()) + (param.index % 256)) & 255));
    }
}

LedStripService::LedStripService(const uint16_t len, const uint8_t p) {
    pin = p;
    set_len(len);
    set_color(0, 0, 0);
    animator = new NeoPixelAnimator(255);
}

LedStripService::~LedStripService() {
    checked_delete(led_strip);
    checked_delete(animator);
}

void LedStripService::set_len(uint16_t len) {
    if (led_strip != NULL) {
        delete led_strip;
    }
    led_strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod>(len, pin);
    led_strip->Begin();
}

uint16_t LedStripService::get_len() const {
    return led_strip->PixelCount();
}

void LedStripService::set_color(const uint16_t h, const uint8_t s, const uint8_t b) {
    hue = (uint16_t) (h % 360);
    saturation = (uint8_t) (s % 101);
    brightness = (uint8_t) (b % 101);
    if (SINGLE_COLOR != mode) {
        set_mode(SINGLE_COLOR);
    } else {
        led_strip->ClearTo(HsbColor(hue, saturation / (float) 100, brightness / (float) 100));
    }
}

uint32_t LedStripService::get_color() const {
    return (uint32_t) ((0xFFFF0000 & hue << 16) | (0x0000FF00 & saturation << 8) | brightness);
}

void LedStripService::set_brightness(const uint8_t b) {
    brightness = b;
    led_strip->ClearTo(HsbColor(hue, saturation / (float) 100, brightness / (float) 100));
}

uint8_t LedStripService::get_brightness() const {
    return brightness;
}

void LedStripService::set_delay(const uint16_t d) {
    animator->setTimeScale(d);
}

uint16_t LedStripService::get_delay() const {
    return animator->getTimeScale();
}

void LedStripService::set_mode(const LED_STRIP_MODE m) {
    switch ((mode = m)) {
        case SINGLE_COLOR:
            set_color(hue, saturation, brightness);
            return;
        case ANIMATION_0:
            animator->StartAnimation(ANIMATION_BEGIN, ANIMATION_END, [this](const AnimationParam &param) {
                animation_0(param);
            });
            break;
        case ANIMATION_1:
            animator->StartAnimation(ANIMATION_BEGIN, ANIMATION_END, [this](const AnimationParam &param) {
                animation_1(param);
            });
            break;
    }
}

LED_STRIP_MODE LedStripService::get_mode() const {
    return mode;
}

void LedStripService::cycle_routine() {
    // Call at least every 10ms
    if (animator->IsAnimating())
        animator->UpdateAnimations();
    if (led_strip != NULL && led_strip->CanShow())
        led_strip->Show();
}
