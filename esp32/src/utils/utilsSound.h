#pragma once
#include <Arduino.h>
#include <math.h>
#include "config.h"
#include "consts.h"
#include "assets/midiNotesInts.h"
#include "types/midi.h"

void logGraphChannelValue(String pre, int32_t val, byte maxStars, bool negatives = false)
{
    Serial.print(pre);

    auto stars = (val * maxStars) >> 16;

    if (negatives)
    {
        // negative side
        for (byte i = 0; i < maxStars; i++)
        {
            if (i >= maxStars + stars)
                Serial.print("*");
            else
                Serial.print("-");
        }
    }

    for (byte i = 0; i < maxStars; i++)
    {
        if (i <= stars)
            Serial.print("*");
        else
            Serial.print("-");
    }
    Serial.println(val);
}

/** basically  sampleTime*freq but mapped to angle space */
FREQ_T calcWaveAngleFromTime(uint32_t sampleTime, FREQ_T freq)
{
    uint32_t angle = uint32_t(freq) * sampleTime;
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
static EnvelopeState nextState(Envelope &envelope, EnvelopeState state)
{
    EnvelopeState newState = state;

    switch (state)
    {
    case EnvelopeState::PRESSED:
        newState = EnvelopeState::ATTACK;
        if (envelope.attack == 0)
        {
            newState = nextState(envelope, newState);
        }
        break;

    case EnvelopeState::ATTACK:
        newState = EnvelopeState::DECAY;
        if (envelope.decay == 0)
        {
            newState = nextState(envelope, newState);
        }
        break;

    case EnvelopeState::DECAY:
        newState = EnvelopeState::SUSTAIN;
        if (envelope.sustain == 0)
        {
            // no sustain! skip all directly to dead (no need to release because the sustain volume is 0)
            newState = EnvelopeState::DEAD;
        }
        break;

    case EnvelopeState::SUSTAIN:
        newState = EnvelopeState::RELEASE;
        if (envelope.release == 0)
        {
            newState = nextState(envelope, newState);
        }
        break;

    case EnvelopeState::RELEASE:
    case EnvelopeState::DEAD:
        newState = EnvelopeState::DEAD;
        break;
    }

    return newState;
}

void updateNoteEnvelope(Note &note, unsigned long now)
{
    // helper var
    static byte max = 0xFF;

    // double size for extra space for calcs
    uint16_t output = 0;

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
        note.state = nextState(note.envelope, note.state);
    }

    // allowed states:

    if (note.state == EnvelopeState::ATTACK)
    {
        if (dt > attack)
            dt = attack;
        // max*(dt/attack)
        output = (uint16_t)max * dt / attack;

        if (dt >= attack)
        {
            note.stateStartTime = now;
            note.state = nextState(note.envelope, note.state);
        }
    }
    else if (note.state == EnvelopeState::DECAY)
    {
        if (dt > decay)
            dt = decay;
        // max - ((max - sustain) * (dt/decay))
        output = (uint16_t)max - (((uint16_t)max - sustain) * dt / decay);

        if (dt >= decay)
        {
            note.stateStartTime = now;
            note.state = nextState(note.envelope, note.state);
        }
    }
    else if (note.state == EnvelopeState::SUSTAIN)
    {
        // = sustain level
        output = sustain;
    }
    else if (note.state == EnvelopeState::RELEASE)
    {
        // note 1: decay from startReleaseAmplitude (in case released before the sustain state)
        // note 2: if no sustain - than use the smaller number of what's-left-of-the-decay or release time! otherwise it's weird no?

        auto stateTime = release;
        if (sustain < 0)
        {
            // calc what's left of the decay (if it began) :
            unsigned long totalNotePlayTime = now - note.noteStartTime;
            uint16_t decayLeft = std::min(
                uint16_t(attack + decay - totalNotePlayTime),
                uint16_t(decay));
            // use the smallest of release or decayLeft
            stateTime = std::min(release, decayLeft);
        }

        if (dt > stateTime)
            dt = stateTime;

        // releaseMaxAmplitude - releaseMaxAmplitude*(dt/stateTime)
        output = releaseMaxAmplitude - ((uint16_t)releaseMaxAmplitude * dt / stateTime);

        if (dt >= stateTime)
        {
            note.stateStartTime = now;
            note.state = nextState(note.envelope, note.state);
            output = 0;
        }
    }
    else if (note.state == EnvelopeState::DEAD)
    {
        output = 0;
    }

    // check and set
    if (output > max)
    {
        output = max;
    }
    note.currentAmplitude = output;
}

void updateNoteRelease(Note &note, unsigned long now)
{
    if (note.state == EnvelopeState::DEAD || note.state == EnvelopeState::RELEASE)
    {
        return;
    }

    note.state = EnvelopeState::RELEASE;
    note.stateStartTime = now;
    note.startReleaseAmplitude = note.currentAmplitude;
}

int16_t fastSigmoid_signed_32_to_16(int32_t val)
{
    if (val > 163835)
        return INT16_MAX;
    else if (val > 131068)
        return (val >> 6) + 29900;
    else if (val > 98301)
        return (val >> 4) + 23756;
    else if (val > 75364)
        return (val >> 3) + 17612;
    else if (val > 32767)
        return (val >> 2) + 8192;
    else if (val > 0)
        return (val >> 1) + 0;

    else if (val == 0)
        return 0;

    return -fastSigmoid_signed_32_to_16(-val);
}