#pragma once

#include <Arduino.h>
#include <math.h>
#include "consts.h"
#include "types/i2sFrame.h"
#include "state/midiState.h"

/**
 * @param angle: (0.0 - WAVE_PI (as 0..2PI))
 * @return range (INT16_MIN - INT16_MAX) of signed int16_t
 */
typedef int16_t (*wave_generator_func_t)(FREQ_T angle, FREQ_T phase);

int16_t wave_sawtooth(FREQ_T angle, FREQ_T phase)
{
    angle = (uint32_t(angle) + phase) % WAVE_PI_2;

    int32_t res;
    res = angle < WAVE_PI
              ? (angle << 1) - INT16_MAX
              : ((WAVE_PI_2 - angle) << 1) - INT16_MAX;

    // no need - same shifts R and L:
    // res = (res << 16) >> SHIFT_WAVE_SAMPLE_RATE; // *waveOutputMax/sampleRate

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