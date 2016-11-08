#include <led_strip.h>

// NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(10, 0);
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> *strip = NULL;
Ticker strip_timer;
uint32_t strip_timer_tick = 0;
strip_modes selected_strip_mode = NONE;
uint16_t j = 0;

void setup_strip(uint8_t pixels) {
  if (strip == NULL)
    strip =
        new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>(pixels, 0);
  // this resets all the neopixels to an off state
  strip->Begin();
  strip->Show();
}

RgbColor Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return RgbColor(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return RgbColor(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return RgbColor(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void rainbow() {
  for (uint16_t i = 0; i < strip->PixelCount(); i++) {
    strip->SetPixelColor(i, Wheel((i + j) & 255));
  }
  strip->Show();
  if (j++ >= 256)
    j = 0;
}

void rainbowCycle() {
  for (uint16_t i = 0; i < strip->PixelCount(); i++) {
    strip->SetPixelColor(i, Wheel(((i * 256 / strip->PixelCount()) + j) & 255));
  }
  strip->Show();
  if (j++ >= 256 * 5)
    j = 0;
}

void plain() {
  for (uint16_t i = 0; i < strip->PixelCount(); i++) {
    strip->SetPixelColor(i, RgbColor(j % 256, j % 256, j % 256));
  }
  strip->Show();
  if (j++ >= 256)
    j = 0;
}

void strip_mode(strip_modes mode, uint32_t delay_time) {
  if (strip_timer_tick == delay_time && selected_strip_mode == mode)
    return;
  strip_timer.detach();
  switch (mode) {
  case RAINBOW:
    strip_timer.attach_ms(delay_time, rainbow);
    break;
  case RAINBOW_CYCLE:
    strip_timer.attach_ms(delay_time, rainbowCycle);
    break;
  case PLAIN:
    strip_timer.attach_ms(delay_time, plain);
    break;
  default:
    break;
  }
  strip_timer_tick = delay_time;
  selected_strip_mode = mode;
}
