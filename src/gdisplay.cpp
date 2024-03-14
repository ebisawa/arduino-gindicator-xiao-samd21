//
// Created by ebisawa on 2024/03/10.
//
//
// Created by ebisawa on 2024/03/08.
//
#include <Seeed_Arduino_FreeRTOS.h>
#include <Adafruit_NeoPixel.h>
#include "main.h"
#include "gdisplay.h"

#define GDISPLAY_HZ  20

static const GDisplayPixel Off      = { 0, 0, 0 };
static const GDisplayPixel Red      = { 255, 0, 0 };
static const GDisplayPixel GreenIM  = { 0, 127, 0 };
static const GDisplayPixel Green    = { 0, 255, 0 };
static const GDisplayPixel Blue     = { 0, 0, 255 };
static const GDisplayPixel YellowIM = { 127, 127, 0 };
static const GDisplayPixel Yellow   = { 255, 255, 0 };
static const GDisplayPixel Magenta  = { 255, 0, 255 };

static const GDisplayColors GreenColorMap[] = {
    { { Off,     Off,     Off,      Off,      Off,      Off,      Off,     Off,    }, BlinkOff },
    { { Green,   Green,   Green,    Green,    Green,    Green,    Green,   Green   }, BlinkOff },
};

static const GDisplayColors GxColorMap[] = {
    { { Off,     Off,     Off,      Off,      Off,      Off,      Off,     Off,    }, BlinkOff },
    { { Off,     Off,     Off,      Off,      Off,      Off,      Off,     Off     }, BlinkOff },  // >= 0.1G
    { { GreenIM, Off,     Off,      Off,      Off,      Off,      Off,     GreenIM }, BlinkOff },  // >= 0.2G
    { { Green,   Off,     Off,      Off,      Off,      Off,      Off,     Green   }, BlinkOff },  // >= 0.3G
    { { Green,   GreenIM, Off,      Off,      Off,      Off,      GreenIM, Green   }, BlinkOff },  // >= 0.4G
    { { Green,   Green,   Off,      Off,      Off,      Off,      Green,   Green   }, BlinkOff },  // >= 0.5G
    { { Green,   Green,   YellowIM, Off,      Off,      YellowIM, Green,   Green   }, BlinkOff },  // >= 0.6G
    { { Green,   Green,   Yellow,   Off,      Off,      Yellow,   Green,   Green   }, BlinkOff },  // >= 0.7G
    { { Green,   Green,   Yellow,   YellowIM, YellowIM, Yellow,   Green,   Green   }, BlinkOff },  // >= 0.8G
    { { Green,   Green,   Yellow,   Yellow,   Yellow,   Yellow,   Green,   Green   }, BlinkOff },  // >= 0.9G
    { { Yellow,  Yellow,  Yellow,   Yellow,   Yellow,   Yellow,   Yellow,  Yellow  }, BlinkOff  },  // >= 1.0G
};

static const GDisplayPixel GyColorMap[][PIXEL_COUNT / 2] = {
    { Off,  Off,   Off,   Off      },
    { Off,  Off,   Off,   Off      },  // >= 0.1G
    { Off,  Off,   Off,   Off      },  // >= 0.2G
    { Off,  Off,   Off,   Off      },  // >= 0.3G
    { Off,  Off,   Off,   Off      },  // >= 0.4G
    { Off,  Blue,  Off,   Off      },  // >= 0.5G
    { Off,  Blue,  Off,   Off      },  // >= 0.6G
    { Off,  Off,   Blue,  Off      },  // >= 0.7G
    { Off,  Off,   Blue,  Off      },  // >= 0.8G
    { Off,  Off,   Off,   Blue     },  // >= 0.9G
    { Off,  Off,   Off,   Magenta  },  // >= 1.0G
};

static const GDisplayPixel GyAlertMap[PIXEL_COUNT / 2] =
    { Off,   Off,   Off,   Red     };


void GDisplayPixel::mix(GDisplayPixel *dest, const GDisplayPixel *next, int k) const {
    dest->r = this->r + (next->r - this->r) * k / 10;
    dest->g = this->g + (next->g - this->g) * k / 10;
    dest->b = this->b + (next->b - this->b) * k / 10;
}

bool GDisplayPixel::is_zero() const {
    return r == 0 && g == 0 && b == 0;
}

void GDisplayColors::mix(GDisplayColors *dest, const GDisplayColors *next, int k) const {
    for (int i = 0; i < PIXEL_COUNT; i++)
        Pixels[i].mix(&dest->Pixels[i], &next->Pixels[i], k);

    dest->Blink = this->Blink;
}

int GDisplay::gk(float g) {
    float ag = abs(g);

    if (ag >= 1.0)
        return 0;

    return ((int) (ag * 100)) % 10;
}

int GDisplay::gindex(float g) {
    float ag = abs(g);

    if (ag >= 1.0)
        return 10;

    return ((int) (ag * 10)) % 10;
}

int GDisplay::gindex_next(int map_index) {
    return (++map_index > 10) ? 10 : map_index;
}

void GDisplay::gx_get_colors(GDisplayColors *colors, float gx) {
    int index0, index1;

    // gx < 0 means deceleration
    if (gx >= 0.0) {
        index0 = 0;
        index1 = 0;
    } else {
        index0 = gindex(gx);
        index1 = gindex_next(index0);
    }

    GxColorMap[index0].mix(colors, &GxColorMap[index1], gk(gx));
}

const GDisplayPixel *GDisplay::gy_get_pixels(float gy) {
    return GyColorMap[gindex(gy)];
}

void GDisplay::np_set_base_colors(const GDisplayColors *colors) {
    const GDisplayPixel *pixels = colors->get_pixels();

    for (int i = 0; i < PIXEL_COUNT; i++)
        NeoPixel.setPixelColor(i, pixels[i].r, pixels[i].g, pixels[i].b);
}

void GDisplay::np_set_overlay_pixels_right(const GDisplayPixel *pixels) {
    int pos = 3;
    for (int i = 0; i < PIXEL_COUNT / 2; i++, pos--) {
        if (!pixels[i].is_zero())
            NeoPixel.setPixelColor(pos, pixels[i].r, pixels[i].g, pixels[i].b);
    }
}

void GDisplay::np_set_overlay_pixels_left(const GDisplayPixel *pixels) {
    int pos = 4;
    for (int i = 0; i < PIXEL_COUNT / 2; i++, pos++) {
        if (!pixels[i].is_zero())
            NeoPixel.setPixelColor(pos, pixels[i].r, pixels[i].g, pixels[i].b);
    }
}

void GDisplay::show() {
    xSemaphoreGive(SemDisplay);
}

void GDisplay::set_green(int k) {
    GDisplayColors colors;

    GreenColorMap[0].mix(&colors, &GreenColorMap[1], k);
    np_set_base_colors(&colors);
    show();
}

void GDisplay::set_gx(float gx) {
    GDisplayColors colors;

    gx_get_colors(&colors, gx);
    np_set_base_colors(&colors);
    BlinkMode = colors.get_blink();
    show();
}

void GDisplay::set_gy(float gy, bool red_alert) {
    const GDisplayPixel *pixels = gy_get_pixels(gy);

    if (gy > 0.0)
        np_set_overlay_pixels_right(pixels);
    if (gy < 0.0)
        np_set_overlay_pixels_left(pixels);

    BlinkMode = (red_alert) ? BlinkOn : BlinkMode;
    show();
}

void GDisplay::set_gxyt(float gx, float gy, float gt) {
    set_gx(gx);
    set_gy(gy, gt >= 1.05);
}

void GDisplay::begin() {
    NeoPixel.begin();
    NeoPixel.setBrightness(PIXEL_BRIGHT);
    NeoPixel.clear();
    NeoPixel.show();

    xTaskCreate(
        display_task,
        "display_task",
        128,
        this,
        2,    /* must be higher than main task */
        nullptr);
}

void GDisplay::clear() {
    NeoPixel.clear();
    show();
}

[[noreturn]] void GDisplay::display_task(void *params) {
    auto *self = (GDisplay *) params;
    static bool blink_state = false;
    static uint8_t blink_pixel[PIXEL_COUNT * PIXEL_BYTES];

    for (;;) {
        xSemaphoreTake(self->SemDisplay, portMAX_DELAY);

        if (!blink_state) {
            self->NeoPixel.show();
            if (self->BlinkMode == BlinkOn) {
                blink_state = true;
                xSemaphoreGive(self->SemDisplay);
            }
        } else {
            memcpy(blink_pixel, self->NeoPixel.getPixels(), sizeof(blink_pixel));
            self->NeoPixel.clear();
            self->NeoPixel.show();
            memcpy(self->NeoPixel.getPixels(), blink_pixel, sizeof(blink_pixel));
            blink_state = false;
            xSemaphoreGive(self->SemDisplay);
        }

        vTaskDelay(pdMS_TO_TICKS(1000 / GDISPLAY_HZ));
    }
}
