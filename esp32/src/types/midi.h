#pragma once

#include <Arduino.h>
#include "assets/midiNotesFloat.h"
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
    float attack;
    float decay;
    float sustain;
    float release;
    float attackOvershoot;

    Envelope()
    {
        this->attack = 0;
        this->decay = 0;
        this->sustain = 0;
        this->release = 0;
        this->attackOvershoot = 1;
    }
    Envelope(
        float attack,
        float decay,
        float sustain,
        float release,
        float attackOvershoot = 1.2)
    {
        this->attack = attack;
        this->decay = decay;
        this->sustain = sustain;
        this->release = release;
        this->attackOvershoot = attackOvershoot;
    }
};

struct Note
{
    byte pitch;
    byte velocity;
    float velocityFactor;
    float freq;
    float angle;
    float phase;
    bool isDown;
    Envelope envelope;
    unsigned long lastEnvelopeUpdateTime;
    EnvelopeState state;
    unsigned long stateStartTime;
    float currentAmplitude;
    float startReleaseAmplitude;

    Note()
    {
        this->pitch = 0;
        this->velocity = 0;
        this->velocityFactor = calcVelocityFactor(0);
        this->freq = MIDI_NOTES_FLOAT[0];
        this->angle = 0;
        this->phase = 0;
        this->isDown = true;
        this->envelope = Envelope();
        this->lastEnvelopeUpdateTime = 0;
        this->state = EnvelopeState::PRESSED;
        this->stateStartTime = 0;
        this->currentAmplitude = 1.0;
        this->startReleaseAmplitude = 1.0;
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
        this->freq = MIDI_NOTES_FLOAT[pitch];
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