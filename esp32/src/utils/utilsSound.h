#pragma once
#include <Arduino.h>
#include "types/midi.h"

float getNoteFrequency(byte pitch, float pitchBend)
{
    if (pitchBend > 1 && pitch < 125)
    {
        auto a = MIDI_NOTES_FLOAT[pitch + 1];
        auto b = MIDI_NOTES_FLOAT[pitch + 2];
        return a + ((b - a) * (pitchBend - 1.0));
    }
    else if (pitchBend >= __FLT_EPSILON__ && pitch < 126)
    {
        auto a = MIDI_NOTES_FLOAT[pitch + 0];
        auto b = MIDI_NOTES_FLOAT[pitch + 1];
        return a + ((b - a) * (pitchBend));
    }
    else if (pitchBend <= -1 && pitch > 2)
    {
        auto a = MIDI_NOTES_FLOAT[pitch - 1];
        auto b = MIDI_NOTES_FLOAT[pitch - 2];
        return a + ((b - a) * (-pitchBend - 1.0));
    }
    else if (pitchBend <= -__FLT_EPSILON__ && pitch > 1)
    {
        auto a = MIDI_NOTES_FLOAT[pitch - 0];
        auto b = MIDI_NOTES_FLOAT[pitch - 1];
        return a + ((b - a) * (-pitchBend));
    }
    else
    {
        return MIDI_NOTES_FLOAT[pitch];
    }
}

//-------------------------------------------------------
void updateNoteEnvelope(Note &note, unsigned long now)
{
    float output = 0;
    unsigned long dt = now - note.stateStartTime;
    float attackOvershoot = note.envelope.attackOvershoot;

    // pressed will initially switch to attack
    // stateStartTime time will stay the pressed time!
    if (note.state == EnvelopeState::PRESSED)
    {
        note.state = EnvelopeState::ATTACK;
    }

    // allowed states:
    if (note.state == EnvelopeState::ATTACK)
    {
        output = attackOvershoot * (dt / note.envelope.attack);

        // no over overshoot just in case
        if (output > attackOvershoot)
        {
            output = attackOvershoot;
        }
        if (dt > note.envelope.attack)
        {
            note.state = EnvelopeState::DECAY;
            note.stateStartTime = now;
        }
    }
    else if (note.state == EnvelopeState::DECAY)
    {
        output = attackOvershoot + (1.0 - attackOvershoot) * (dt / note.envelope.decay);
        if (dt > note.envelope.decay)
        {
            note.state = EnvelopeState::SUSTAIN;
            note.stateStartTime = now;
        }
    }
    else if (note.state == EnvelopeState::SUSTAIN)
    {
        output = 1.0;
        // negative sustain means no note decay (stays forever)
        if (dt > note.envelope.sustain)
        {
            if (note.envelope.sustain > 0)
            {
                note.state = EnvelopeState::RELEASE;
                note.startReleaseAmplitude = output;
                note.stateStartTime = now;
            }
        }
    }
    else if (note.state == EnvelopeState::RELEASE)
    {
        // decay from startReleaseAmplitude (in case released before the sustain state)
        // fake start time

        output = note.startReleaseAmplitude - (dt / note.envelope.release);

        if (dt > note.envelope.release)
        {
            note.state = EnvelopeState::DEAD;
            note.stateStartTime = now;
            output = 0.0;
        }
    }
    else if (note.state == EnvelopeState::DEAD)
    {
        output = 0.0;
    }

    // no negative
    if (output < 0)
    {
        output = 0;
    }

    // velocity:
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