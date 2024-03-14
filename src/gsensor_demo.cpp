//
// Created by ebisawa on 2024/03/10.
//
#include "main.h"
#include "gsensor_demo.h"

static const int8_t DemoData[][2] = {
    {   0,    0 },

    // 0.0s
    {   0,    0 },
    {  -10,   2 },
    {  -30,   5 },
    {  -50,   7 },
    {  -65,  10 },

    // 0.5s
    {  -74,  12 },
    {  -85,  15 },
    {  -95,  12 },
    { -101,  16 },
    {  -98,  20 },

    // 1.0s
    {  -97,  20 },
    {  -95,  21 },
    {  -92,  25 },
    {  -88,  32 },
    {  -79,  38 },

    // 1.5s
    {  -70,  45 },
    {  -65,  55 },
    {  -60,  60 },
    {  -55,  65 },
    {  -50,  75 },

    // 2.0s
    {  -45,  80 },
    {  -38,  85 },
    {  -35,  88 },
    {  -32,  95 },
};


void GSensorDemo::begin() {
    index = 0;
}

void GSensorDemo::fetch() {
    if (index == 0)
        delay(1000);

    index++;
    if (index >= NELEMS(DemoData))
        index = 0;
}

float GSensorDemo::gx() const {
    return DemoData[index][0] / 100.0;
}

float GSensorDemo::gy() const {
    return DemoData[index][1] / 100.0;
}

float GSensorDemo::gtotal() const {
    return sqrt(gx() * gx() + gy() * gy());
}
