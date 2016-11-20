#include <led_strip.h>

RgbColor wheel(byte pos) {
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
