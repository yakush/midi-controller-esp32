#pragma once

#include <Arduino.h>
#include <math.h>
#include "consts.h"
#include "types/i2sFrame.h"
#include "state/midiState.h"

/**
 * @param angle: (0.0 - 2PI)
 * @return range 0.0 - 1.0
 */
typedef float (*wave_generator_func_t)(float angle, float phase);

float wave_sawtooth(float angle, float phase)
{
    angle = angle + phase;
    if (angle > PI_2)
    {
        angle -= PI_2;
    }
    else if (angle < 0)
    {
        angle += PI_2;
    }
    return (angle < PI ? (angle - PI_1_2) : (PI_3_2 - angle)) / PI;
}