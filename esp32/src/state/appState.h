#pragma once

#include <Arduino.h>
#include <mutex>
#include "state.h"

class AppState_Class
{
private:
    STATE_LOCK_DEFINE(_mutex);
    byte _volume = 30;
    unsigned long _i2s_writeTime = 0;

public:
    byte volume() { return _volume; }
    void volume(byte value)
    {
        STATE_LOCK(_mutex);
        _volume = value;
        STATE_UNLOCK(_mutex);
    }

    unsigned long i2s_writeTime() { return _i2s_writeTime; }
    void i2s_writeTime(unsigned long value)
    {
        STATE_LOCK(_mutex);
        _i2s_writeTime = (_i2s_writeTime * 99 + value * 1) / 100;
        STATE_UNLOCK(_mutex);
    }
};

AppState_Class AppState;