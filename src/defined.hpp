#ifndef DEFINE_H
#define DEFINE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// LVGL - Display
#include "Display/Display.hpp"
Display displayDevice(Serial);

// DJI - Battery
#include "Battery/DJIBattery.hpp"
DJIBattery batteryDevice(Serial1,0);

#endif // DEFINE_H