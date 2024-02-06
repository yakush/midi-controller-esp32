#pragma once
#include <Arduino.h>
#include "types/midi.h"

//-------------------------------------------------------
static FRAME_CHANNEL_T mapVal(int v, FRAME_CHANNEL_T fromMin, FRAME_CHANNEL_T fromMax, FRAME_CHANNEL_T toMin, FRAME_CHANNEL_T toMax, bool limit = true)
{
    FRAME_CHANNEL_DOUBLE_T fromRange = fromMax - fromMin;
    FRAME_CHANNEL_DOUBLE_T toRange = toMax - toMin;
    FRAME_CHANNEL_DOUBLE_T res = toMin + (toRange * ((v - fromMin) / fromRange));

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

FRAME_CHANNEL_T calcVelocityFactor(byte velocity)
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
    static FRAME_CHANNEL_T vals[]{
        (FRAME_CHANNEL_T)(0.20 * FRAME_CHANNEL_MAX), // pp
        (FRAME_CHANNEL_T)(0.50 * FRAME_CHANNEL_MAX), // p
        (FRAME_CHANNEL_T)(0.80 * FRAME_CHANNEL_MAX), // norm
        (FRAME_CHANNEL_T)(0.92 * FRAME_CHANNEL_MAX), // f
        (FRAME_CHANNEL_T)(0.98 * FRAME_CHANNEL_MAX), // f
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
    return FRAME_CHANNEL_MAX;
}
