#pragma once

#include <Arduino.h>
#include <math.h>
#include "consts.h"
#include "types/i2sFrame.h"
#include "state/midiState.h"

/**
 * @param angle: (0.0 - WAVE_PI (as 0..2PI))
 * @return range 0.0 - 1.0
 */
typedef FRAME_CHANNEL_T (*wave_generator_func_t)(FREQ_T angle, FREQ_T phase);

FRAME_CHANNEL_T wave_sawtooth(FREQ_T angle, FREQ_T phase)
{
    angle = (uint32_t(angle) + phase) % WAVE_PI_2;

    FRAME_CHANNEL_DOUBLE_T res;
    res = angle < WAVE_PI
              ? (angle << 1)
              : (WAVE_PI_2 - angle) << 1;
    res = (res << SHIFT_FRAME_CHANNEL) >> SHIFT_WAVE_SAMPLE_RATE; // *channelMax/sampleRate
    return res;

    // return ((angle < PI ? (angle - PI_1_2) : (PI_3_2 - angle)) / PI);

    // angle = angle + phase;
    // if (angle > PI_2)
    // {
    //     angle -= PI_2;
    // }
    // else if (angle < 0)
    // {
    //     angle += PI_2;
    // }

    // return FRAME_CHANNEL_MAX * ((angle < PI ? (angle) : (PI - angle)) / PI);
    // // return ((angle < PI ? (angle - PI_1_2) : (PI_3_2 - angle)) / PI);
}