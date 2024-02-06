#pragma once

#include <Arduino.h>

struct __attribute__((packed)) I2S_Frame
{
    int16_t channel1;
    int16_t channel2;

    I2S_Frame(int v = 0)
    {
        channel1 = channel2 = v;
    }

    I2S_Frame(int ch1, int ch2)
    {
        channel1 = ch1;
        channel2 = ch2;
    }
};