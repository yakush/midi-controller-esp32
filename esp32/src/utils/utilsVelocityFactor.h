#pragma once
#include <Arduino.h>
#include "types/midi.h"

//-------------------------------------------------------
static byte mapVal(int v, byte fromMin, byte fromMax, byte toMin, byte toMax, bool limit = true)
{
    uint16_t fromRange = fromMax - fromMin;
    uint16_t toRange = toMax - toMin;
    uint16_t res = toMin + (toRange * ((v - fromMin) / fromRange));

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

byte calcVelocityFactor(byte velocity)
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
    static byte vals[]{
        (byte)(0.20 * 0xFF), // pp
        (byte)(0.50 * 0xFF), // p
        (byte)(0.80 * 0xFF), // norm
        (byte)(0.92 * 0xFF), // f
        (byte)(0.98 * 0xFF), // f
    };

    // calc:
    float v = velocity;

    // bellow min stop
    if (v < stops[0])
    {
        return vals[0];
    }
    for (size_t i = 1; i < numStops; i++)
    {
        if (v < stops[i])
        {
            return mapVal(v, stops[i - 1], stops[i], vals[i - 1], vals[i]);
        }
    }

    // over max stop
    return 0xFF;
}
