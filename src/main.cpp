//
// Created by ebisawa on 2024/03/08.
//
#include <Seeed_Arduino_FreeRTOS.h>
#include <I2Cdev.h>
#include "main.h"
#include "gdisplay.h"
#include "gsensor.h"
#include "gsensor_demo.h"

#define MAIN_HZ     10

#define DISP_PIN    3
#define DISP_TYPE   (NEO_GRB + NEO_KHZ800)

static GDisplay Disp = GDisplay(DISP_PIN, DISP_TYPE);
static GSensor Sensor = GSensor();
//static GSensorDemo Sensor = GSensorDemo();

static volatile SemaphoreHandle_t SemDemoFinish;

[[noreturn]] static void init_task(void *params);
[[noreturn]] static void demo_task(void *params);
[[noreturn]] static void main_task(void *params);

void setup() {
    Wire.begin();
    Wire.setClock(400000);
    Disp.begin();

    xTaskCreate(
        init_task,
        "init_task",
        512,
        nullptr,
        0,
        nullptr);

    vTaskStartScheduler();
}

void loop() { /*nop*/ }


[[noreturn]] static void init_task(void *params) {
    xTaskHandle handle;

    SemDemoFinish = xSemaphoreCreateBinary();

    /* show demo pattern while sensor initialization going */
    xTaskCreate(
        demo_task,
        "dmeo_task",
        256,
        nullptr,
        0,
        &handle);

    /* initialize in background */
    Sensor.begin();

    /* wait until opening demo finished */
    xSemaphoreTake(SemDemoFinish, portMAX_DELAY);

    /* start main task */
    xTaskCreate(
        main_task,
        "main_task",
        256,
        nullptr,
        1,
        nullptr);

    vTaskDelete(handle);
    vTaskDelete(nullptr);
    Disp.clear();
}

[[noreturn]] static void demo_task(void *params) {
    /* right */
    for (float gy = 0.0; gy <= 1.2;) {
        Disp.clear();
        Disp.set_gy(gy);
        gy += 0.01;
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    /* left */
    for (float gy = 0.0; gy >= -1.2;) {
        Disp.clear();
        Disp.set_gy(gy);
        gy -= 0.01;
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    for (;;) {
        /* green */
        for (int j = 0; j < 2; j++) {
            for (int i = 0; i <= 10; i++) {
                Disp.set_green(i);
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            for (int i = 10; i >= 0; i -= 2) {
                Disp.set_green(i);
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            vTaskDelay(pdMS_TO_TICKS(50));
        }

        xSemaphoreGive(SemDemoFinish);
    }
}

[[noreturn]] static void main_task(void *params) {
    float gx, gy, gt;
    TickType_t start_time;

    for (;;) {
        start_time = xTaskGetTickCount();

        Sensor.fetch();
        gx = Sensor.gx();
        gy = Sensor.gy();
        gt = Sensor.gtotal();

#if 0
        Serial.print("gx=");
        Serial.println(gx);
        Serial.print("gy=");
        Serial.println(gy);
        Serial.print("total=");
        Serial.println(gt);
#endif

        Disp.set_gxyt(gx, gy, gt >= 0.85);

        vTaskDelayUntil(&start_time, pdMS_TO_TICKS(1000 / MAIN_HZ));
    }
}
