#pragma once

#include <Arduino.h>
#include <math.h>
#include "consts.h"
#include "types/i2sFrame.h"
#include "state/midiState.h"
#include "utils/waveInterpolator.h"

enum WaveType{
    SIN,
    SAWTOOTH,
};

class WaveGenerator
{
public:
    WaveGenerator() {}
    virtual ~WaveGenerator() {}

    /**
     * @param angle: (0.0 - WAVE_PI (as 0..2PI))
     * @return range (INT16_MIN - INT16_MAX) of signed int16_t
     */
    virtual int16_t calc(uint16_t angle) = 0;
};

//-------------------------------------------------------
//-------------------------------------------------------
class SawtoothWaveGenerator : public WaveGenerator
{
public:
    SawtoothWaveGenerator()
    {
    }
    ~SawtoothWaveGenerator()
    {
    }
    int16_t calc(uint16_t angle) override
    {

        int32_t res;
        res = angle < WAVE_PI
                  ? (angle << 1) - INT16_MAX
                  : ((WAVE_PI_2 - angle) << 1) - INT16_MAX;

        // no need - same shifts R and L:
        // res = (res << 16) >> SHIFT_WAVE_SAMPLE_RATE; // *waveOutputMax/sampleRate

        return res;
    }
};
//-------------------------------------------------------
//-------------------------------------------------------

class SinWaveGenerator : public WaveGenerator
{

private:
    WaveInterpolator *interpolator;

public:
    SinWaveGenerator(size_t resolution)
    {
        interpolator = new WaveInterpolator(
            resolution,
            [](uint16_t x) -> int16_t
            {
                float angle = (float)x / UINT16_MAX;
                return INT16_MAX * sinf(PI * 2 * angle);
            });
    }
    ~SinWaveGenerator()
    {
        delete interpolator;
    }

    int16_t calc(uint16_t angle)
    {
        return interpolator->calc(angle);
    }
};