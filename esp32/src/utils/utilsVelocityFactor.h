#pragma once
#include <Arduino.h>
#include "types/midi.h"

//-------------------------------------------------------
static float mapVal(float v, float fromMin, float fromMax, float toMin, float toMax, bool limit = true)
{
    float fromRange = fromMax - fromMin;
    float toRange = toMax - toMin;
    float res = toMin + (toRange * ((v - fromMin) / fromRange));

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

float calcVelocityFactor(byte velocity)
{

    // define graph:
    static size_t numStops = 5;
    static float stops[]{
        10,  // pp
        20,  // p
        40,  // norm
        70,  // f
        100, // f
    };
    static float vals[]{
        0.20, // pp
        0.50, // p
        0.80, // norm
        0.92, // f
        0.98, // f
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
    return 1;
}
