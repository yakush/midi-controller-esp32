#pragma once

#include <Arduino.h>
#include "config.h"
#include "assets/midiNotesInts.h"
#include "utils/utilsVelocityFactor.h"

enum EnvelopeState
{
    PRESSED,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    DEAD,
};

struct Envelope
{
    /** time for attack */
    uint16_t attack;
    /** time for decay */
    uint16_t decay;
    /** LEVEL of sustain (decay to this level) */
    byte sustain;
    /** time for release */
    uint16_t release;

    Envelope()
    {
        this->attack = 0;
        this->decay = 0;
        this->sustain = 0;
        this->release = 0;
    }
    Envelope(
        FRAME_CHANNEL_T attack,
        FRAME_CHANNEL_T decay,
        float sustainNormalized,
        FRAME_CHANNEL_T release)
    {
        this->attack = attack;
        this->decay = decay;
        this->sustainNormalized(sustainNormalized);
        this->release = release;
    }

    void sustainNormalized(float val)
    {
        this->sustain = (byte)(val * 0xFF);
    }
    float sustainNormalized()
    {
        return (float)sustain / 0xFF;
    }
};

struct Note
{
    byte pitch;
    byte velocity;
    byte velocityFactor;
    FREQ_T freq;
    FREQ_T phase;
    bool isDown;
    Envelope envelope;
    EnvelopeState state;
    unsigned long noteStartTime;
    unsigned long stateStartTime;
    byte currentAmplitude;
    byte startReleaseAmplitude;

    Note()
    {
        this->pitch = 0;
        this->velocity = 0;
        this->velocityFactor = calcVelocityFactor(0);
        this->freq = MIDI_NOTES[0];
        this->phase = 0;
        this->isDown = true;
        this->envelope = Envelope();
        this->state = EnvelopeState::PRESSED;
        this->noteStartTime = 0;
        this->stateStartTime = 0;
        this->currentAmplitude = 0xFF;
        this->startReleaseAmplitude = 0xFF;
    }

    Note(
        byte pitch,
        byte velocity,
        unsigned long startTime,
        Envelope &envelope) : Note()
    {
        this->pitch = pitch;
        this->velocity = velocity;
        this->velocityFactor = calcVelocityFactor(velocity);
        this->freq = MIDI_NOTES[pitch];
        this->envelope = envelope;
        this->noteStartTime = startTime;
        this->stateStartTime = startTime;
    }

    Note(
        byte pitch,
        byte velocity,
        Envelope &envelope) : Note(pitch,
                                   velocity,
                                   millis(),
                                   envelope)
    {
    }
};