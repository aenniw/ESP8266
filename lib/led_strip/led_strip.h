#ifndef LEDSTRIPS_H_
#define LEDSTRIPS_H_
//#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>
#include <Ticker.h>

typedef enum { RAINBOW, RAINBOW_CYCLE, PLAIN, NONE } strip_modes;

void setup_strip(uint8_t pixels);

void strip_mode(strip_modes mode, uint32_t delay);

#endif /* LEDSTRIPS_H_ */
