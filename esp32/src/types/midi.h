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
    FRAME_CHANNEL_T attack;
    FRAME_CHANNEL_T decay;
    FRAME_CHANNEL_T sustain;
    FRAME_CHANNEL_T release;
    FRAME_CHANNEL_T decayLevel;

    Envelope()
    {
        this->attack = 0;
        this->decay = 0;
        this->sustain = 0;
        this->release = 0;
        this->decayLevel = FRAME_CHANNEL_MAX; // no decay
    }
    Envelope(
        FRAME_CHANNEL_T attack,
        FRAME_CHANNEL_T decay,
        FRAME_CHANNEL_T sustain,
        FRAME_CHANNEL_T release,
        float decayLevelNormalized = 0.8)
    {
        this->attack = attack;
        this->decay = decay;
        this->sustain = sustain;
        this->release = release;
        this->decayLevel = decayLevel;
        this->decayLevel = (FRAME_CHANNEL_T)(decayLevelNormalized * FRAME_CHANNEL_MAX);
    }
};

struct Note
{
    byte pitch;
    byte velocity;
    FRAME_CHANNEL_T velocityFactor;
    FREQ_T freq;
    FREQ_T angle;
    FREQ_T phase;
    bool isDown;
    Envelope envelope;
    unsigned long lastEnvelopeUpdateTime;
    EnvelopeState state;
    unsigned long stateStartTime;
    FRAME_CHANNEL_T currentAmplitude;
    FRAME_CHANNEL_T startReleaseAmplitude;

    Note()
    {
        this->pitch = 0;
        this->velocity = 0;
        this->velocityFactor = calcVelocityFactor(0);
        this->freq = MIDI_NOTES[0];
        this->angle = 0;
        this->phase = 0;
        this->isDown = true;
        this->envelope = Envelope();
        this->lastEnvelopeUpdateTime = 0;
        this->state = EnvelopeState::PRESSED;
        this->stateStartTime = 0;
        this->currentAmplitude = UINT16_MAX;
        this->startReleaseAmplitude = UINT16_MAX;
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