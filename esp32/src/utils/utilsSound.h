#pragma once
#include <Arduino.h>
#include "config.h"
#include "consts.h"
#include "assets/midiNotesInts.h"
#include "types/midi.h"

void logGraphChannelValue(String pre, FRAME_CHANNEL_T val, byte maxStars)
{
    auto stars = (FRAME_CHANNEL_DOUBLE_T)val * maxStars / FRAME_CHANNEL_MAX;

    Serial.print(pre);

    for (byte i = 0; i < stars; i++)
    {
        Serial.print("*");
    }
    Serial.println();
}

/** basically  sampleTime*freq but mapped to angle space */
FREQ_T calcWaveAngleFromTime(uint32_t sampleTime, FREQ_T freq)
{
    uint32_t angle = uint32_t(freq) * sampleTime;
    angle = (angle << SHIFT_WAVE_SAMPLE_RATE) >> SHIFT_FRAME_CHANNEL;
    angle = angle % WAVE_PI_2;
    return angle;
}

FREQ_T getNoteFrequency(byte pitch, float pitchBend)
{
    if (pitchBend > 1 && pitch < 125)
    {
        auto a = MIDI_NOTES[pitch + 1];
        auto b = MIDI_NOTES[pitch + 2];
        return a + ((b - a) * (pitchBend - 1.0));
    }
    else if (pitchBend >= __FLT_EPSILON__ && pitch < 126)
    {
        auto a = MIDI_NOTES[pitch + 0];
        auto b = MIDI_NOTES[pitch + 1];
        return a + ((b - a) * (pitchBend));
    }
    else if (pitchBend <= -1 && pitch > 2)
    {
        auto a = MIDI_NOTES[pitch - 1];
        auto b = MIDI_NOTES[pitch - 2];
        return a + ((b - a) * (-pitchBend - 1.0));
    }
    else if (pitchBend <= -__FLT_EPSILON__ && pitch > 1)
    {
        auto a = MIDI_NOTES[pitch - 0];
        auto b = MIDI_NOTES[pitch - 1];
        return a + ((b - a) * (-pitchBend));
    }
    else
    {
        return MIDI_NOTES[pitch];
    }
}

//-------------------------------------------------------
void updateNoteEnvelope(Note &note, unsigned long now)
{
    // helper var
    static FRAME_CHANNEL_T max = FRAME_CHANNEL_MAX;

    FRAME_CHANNEL_T output = 0;

    auto dt = now - note.stateStartTime;

    auto attack = note.envelope.attack;
    auto decay = note.envelope.decay;
    auto sustain = note.envelope.sustain;
    auto release = note.envelope.release;

    auto releaseMaxAmplitude = note.startReleaseAmplitude;

    // pressed will initially switch to attack
    // stateStartTime time will stay the pressed time!
    if (note.state == EnvelopeState::PRESSED)
    {
        note.state = EnvelopeState::ATTACK;
    }

    // allowed states:

    if (note.state == EnvelopeState::ATTACK)
    {
        // max*(dt/attack)
        output = (FRAME_CHANNEL_DOUBLE_T)max * dt / attack;

        if (dt > attack)
        {
            note.stateStartTime = now;
            note.state = EnvelopeState::DECAY;
        }
    }
    else if (note.state == EnvelopeState::DECAY)
    {
        // max - ((max - sustain) * (dt/decay))
        output = (FRAME_CHANNEL_DOUBLE_T)max - (((FRAME_CHANNEL_DOUBLE_T)max - sustain) * dt / decay);

        if (dt > decay)
        {
            note.stateStartTime = now;

            // if sustain is 0 - go directly to DEAD
            note.state = sustain > 0
                             ? EnvelopeState::SUSTAIN
                             : EnvelopeState::DEAD;
        }
    }
    else if (note.state == EnvelopeState::SUSTAIN)
    {
        // = sustain level
        output = sustain;
    }
    else if (note.state == EnvelopeState::RELEASE)
    {
        // decay from startReleaseAmplitude (in case released before the sustain state)

        // releaseMaxAmplitude - releaseMaxAmplitude*(dt/release)
        output = releaseMaxAmplitude - ((FRAME_CHANNEL_DOUBLE_T)releaseMaxAmplitude * dt / release);

        if (dt > release)
        {
            note.stateStartTime = now;
            note.state = EnvelopeState::DEAD;
            output = 0;
        }
    }
    else if (note.state == EnvelopeState::DEAD)
    {
        output = 0;
    }

    // // no negative
    // if (output < 0)
    // {
    //     output = 0;
    // }

    // set:
    note.currentAmplitude = output;
}

//-------------------------------------------------------

/**
 *
 */
float normalizeBetween(float val, float min, float max)
{
    float range = (max - min);

    val -= min;
    val = val / range;
    val = (val - floorf(val));
    val *= range;
    val += min;

    return val;
}