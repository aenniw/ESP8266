#ifndef ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_H
#define ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_H

#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>
#include <commons.h>

typedef enum {
    SINGLE_COLOR, ANIMATION_0, ANIMATION_1
} LED_STRIP_MODE;

class LedStripService : public Service {
private:
    HsbColor color = HsbColor(0, 0, 0);
    LED_STRIP_MODE mode = SINGLE_COLOR;
    // TODO: somehow generify this mess
    // The NeoEsp8266Dma800KbpsMethod only supports the RDX0/GPIO3 pin.
    // NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> *led_strip = NULL;

    // NeoEsp8266Uart800KbpsMethod only supports the TXD1/GPIO2 pin. The Pin argument is omitted.
    NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> *led_strip = NULL;

    // NeoEsp8266BitBang800KbpsMethod supports any available pin between 0 and 15. WIFI is not usable
    // NeoPixelBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> *led_strip = NULL;
    NeoPixelAnimator *animator = NULL; // NeoPixel animation management object
protected:
    void animation_0(const AnimationParam &param);

    void animation_1(const AnimationParam &param);

public:
    LedStripService(const uint16_t len);

    void set_len(uint16_t);

    uint16_t get_len() const;

    void set_color(const uint8_t, const uint8_t, const uint8_t);

    uint32_t get_color() const;

    void set_brightness(const uint8_t);

    uint8_t get_brightness() const;

    void cycle_routine();

    void set_delay(const uint16_t);

    uint16_t get_delay() const;

    void set_mode(const LED_STRIP_MODE);

    LED_STRIP_MODE get_mode() const;

    ~LedStripService();
};

#endif //ESP8266_PROJECTS_ROOT_SERVICE_LED_STRIP_H
