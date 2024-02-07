#pragma once

#include <Arduino.h>
#include <mutex>
#include "state.h"

class AppState_Class
{
private:
    STATE_LOCK_DEFINE(_mutex);

    bool _ready = false;

    unsigned long _test_i2s_writeTime = 0;

public:
    bool isReady() { return _ready; }
    void ready(bool value)
    {
        STATE_LOCK(_mutex);
        _ready = value;
        STATE_UNLOCK(_mutex);
    }

    unsigned long i2s_writeTime() { return _test_i2s_writeTime; }
    void i2s_writeTime(unsigned long value)
    {
        STATE_LOCK(_mutex);
        _test_i2s_writeTime = (_test_i2s_writeTime * 99 + value * 1) / 100;
        STATE_UNLOCK(_mutex);
    }
};

AppState_Class AppState;