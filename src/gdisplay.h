//
// Created by ebisawa on 2024/03/10.
//

#ifndef GSENSOR_LED_GDISPLAY_H
#define GSENSOR_LED_GDISPLAY_H

#include <Adafruit_NeoPixel.h>

#define PIXEL_COUNT    8
#define PIXEL_BYTES    3
#define PIXEL_BRIGHT   50

enum GDisplayBlink {
    BlinkOff = 0,
    BlinkOn,
};

struct GDisplayPixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    void mix(GDisplayPixel *dest, const GDisplayPixel *next, int k) const;
    bool is_zero() const;
};

struct GDisplayColors {
    GDisplayPixel Pixels[PIXEL_COUNT];
    GDisplayBlink Blink;

    const GDisplayPixel *get_pixels() const { return Pixels; }
    GDisplayBlink get_blink() const { return Blink; }
    void mix(GDisplayColors *dest, const GDisplayColors *next, int k) const;
};

class GDisplay {
    Adafruit_NeoPixel NeoPixel;
    GDisplayBlink BlinkMode = BlinkOff;
    volatile xSemaphoreHandle SemDisplay;

    static int gk(float g);
    static int gindex(float g);
    static int gindex_next(int map_index);

    static void gx_get_colors(GDisplayColors *colors, float gx);
    static const GDisplayPixel *gy_get_pixels(float gy);

    void np_set_base_colors(const GDisplayColors *colors);
    void np_set_overlay_pixels_right(const GDisplayPixel *pixels);
    void np_set_overlay_pixels_left(const GDisplayPixel *pixels);
    void show();

    [[noreturn]] static void display_task(void *params);

public:
    GDisplay(int16_t pin, neoPixelType type) : NeoPixel(PIXEL_COUNT, pin,  type) {
        SemDisplay = xSemaphoreCreateBinary();
    }

    void set_green(int k);
    void set_gx(float gx);
    void set_gy(float gy, bool red_alert = false);
    void set_gxyt(float gx, float gy, float gt);

    void begin();
    void clear();
};

#endif //GSENSOR_LED_GDISPLAY_H
