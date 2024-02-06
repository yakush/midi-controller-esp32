#pragma once

#include <Arduino.h>
#include "config.h"

struct __attribute__((packed)) I2S_Frame
{
    FRAME_CHANNEL_T channel1;
    FRAME_CHANNEL_T channel2;

    I2S_Frame(FRAME_CHANNEL_T v = 0)
    {
        channel1 = channel2 = v;
    }

    I2S_Frame(FRAME_CHANNEL_T ch1, FRAME_CHANNEL_T ch2)
    {
        channel1 = ch1;
        channel2 = ch2;
    }
};