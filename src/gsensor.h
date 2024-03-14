//
// Created by ebisawa on 2024/03/10.
//

#ifndef GSENSOR_LED_GSENSOR_H
#define GSENSOR_LED_GSENSOR_H

#include "MPU6050_6Axis_MotionApps612_mod.h"

class GSensor {
    MPU6050 mpu;

    Quaternion Q;
    VectorInt16 Acc;
    VectorInt16 AccReal;
    VectorFloat Gravity;

public:
    void begin();
    void fetch();

    float gx() const;
    float gy() const;
    float gtotal() const;
};

#endif //GSENSOR_LED_GSENSOR_H
