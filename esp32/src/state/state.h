#pragma once

#include <mutex>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

//#define STATE_LOCK_DEFINE(x) std::mutex x
#define STATE_LOCK_DEFINE(x) std::recursive_mutex x
#define STATE_LOCK(x) x.lock()
#define STATE_UNLOCK(x) x.unlock()

//#define STATE_LOCK(x) std::lock_guard(x)
//#define STATE_UNLOCK(x) 

// #define STATE_LOCK_DEFINE(x) SemaphoreHandle_t x = xSemaphoreCreateMutex()
// #define STATE_LOCK(x) xSemaphoreTake(x, portMAX_DELAY)
//  #define STATE_UNLOCK(x) xSemaphoreGive(x)
