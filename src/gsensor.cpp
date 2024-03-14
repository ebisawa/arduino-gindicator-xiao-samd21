//
// Created by ebisawa on 2024/03/10.
//
#include "gsensor.h"

static uint8_t Buffer[64];

void GSensor::begin() {
    mpu.initialize();
    mpu.dmpInitialize();
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.setDMPEnabled(true);
}

void GSensor::fetch() {
    if (mpu.dmpGetCurrentFIFOPacket(Buffer)) {
        mpu.dmpGetQuaternion(&Q, Buffer);
        mpu.dmpGetAccel(&Acc, Buffer);
        mpu.dmpGetGravity(&Gravity, &Q);
        mpu.dmpGetLinearAccel(&AccReal, &Acc, &Gravity);
    }
}

float GSensor::gx() const {
    float v = sqrt(AccReal.x * AccReal.x + AccReal.z * AccReal.z) / 8192.0;
    if (AccReal.x < -0)
        v *= -1;

    return v;
}

float GSensor::gy() const {
    return AccReal.y / 8192.0;
}

float GSensor::gtotal() const {
    return sqrt(AccReal.x * AccReal.x + AccReal.y * AccReal.y + AccReal.z * AccReal.z) / 8192.0;
}