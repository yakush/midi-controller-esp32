#pragma once
#include <Arduino.h>
#include "types/midi.h"

//-------------------------------------------------------
static uint16_t mapVal(int v, byte fromMin, byte fromMax, uint16_t toMin, uint16_t toMax, bool limit = true)
{
    uint16_t fromRange = fromMax - fromMin;
    uint32_t toRange = toMax - toMin;
    uint32_t res = toMin + (toRange * ((v - fromMin) / fromRange));

    if (limit && res < toMin)
    {
        res = toMin;
    }
    else if (limit && res > toMax)
    {
        res = toMax;
    }
    return res;
}

uint16_t calcVelocityFactor(byte velocity)
{

    // define graph:
    static size_t numStops = 5;
    static byte stops[]{
        10,  // pp
        20,  // p
        40,  // norm
        70,  // f
        100, // f
    };
    static uint16_t vals[]{
        (uint16_t)(0.20 * 0xFFFF), // pp
        (uint16_t)(0.50 * 0xFFFF), // p
        (uint16_t)(0.80 * 0xFFFF), // norm
        (uint16_t)(0.92 * 0xFFFF), // f
        (uint16_t)(0.98 * 0xFFFF), // f
    };

    // bellow min stop
    if (velocity < stops[0])
    {
        return vals[0];
    }
    for (size_t i = 1; i < numStops; i++)
    {
        if (velocity < stops[i])
        {
            return mapVal(velocity, stops[i - 1], stops[i], vals[i - 1], vals[i]);
        }
    }

    // over max stop
    return 0xFFFF;
}
