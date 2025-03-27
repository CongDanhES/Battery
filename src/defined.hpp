#ifndef DEFINE_H
#define DEFINE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// LVGL - Display
#include "Display/Display.hpp"
Display displayDevice(Serial);

#endif // DEFINE_H