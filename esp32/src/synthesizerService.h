#pragma once

#include <Arduino.h>
#include <vector>

#include "config.h"
#include "logger.h"
#include "assets/midiNotesFloat.h"
#include "types/midi.h"
#include "types/i2sFrame.h"
#include "state/midiState.h"
#include "types/midi.h"
#include "consts.h"
#include "utils/utilsSound.h"
#include "waveGenerators.h"
#include "state/state.h"

//-------------------------------------------------------
//-------------------------------------------------------

STATE_LOCK_DEFINE(lock_wave);
WaveGenerator *wave;

class NotesRunner : public NotesIterator
{
public:
    int32_t output = 0;
    uint32_t sampleTime = 0;

    float lastEnvelopeUpdateMillis = 0;
    bool requestEnvelopeUpdate = false;

    NotesRunner()
    {
    }

    ~NotesRunner()
    {
    }

    bool before(size_t len) override
    {
        unsigned long now = millis();

        if (now > this->lastEnvelopeUpdateMillis + UPDATE_ENVELOPE_TIME || now < this->lastEnvelopeUpdateMillis /*overflow*/)
        {
            this->requestEnvelopeUpdate = true;
            this->lastEnvelopeUpdateMillis = now;
        }
        else
        {
            this->requestEnvelopeUpdate = false;
        }
        return true;
    }

    void after(size_t len) override
    {
        this->requestEnvelopeUpdate = false;
    }

    bool run(Note &note, size_t i, size_t len) override
    {
        // abort dead notes
        if (note.state == EnvelopeState::DEAD)
        {
            return true;
        }

        // just in case note is never updated (i.e playing a new note before loop() ran):
        if (
            this->requestEnvelopeUpdate ||
            note.state == EnvelopeState::PRESSED)
        {
            updateNoteEnvelope(note, millis());
        }

        // generate wave and add to output
        // angle is (0..WAVE_PI_2) as in range of (0..pi)
        FREQ_T freq = note.freq;
        FREQ_T angle = calcWaveAngleFromTime(sampleTime, freq);
        FREQ_T phase = note.phase;
        angle = (uint32_t(angle) + phase) % WAVE_PI_2;

        // build wave
        int32_t noteOutput = 0;

        // saw tooth for now
        // noteOutput = wave_sawtooth(angle, phase);
        noteOutput = wave->calc(angle);

        // envelope:
        noteOutput = (noteOutput * note.currentAmplitude * note.velocityFactor) >> 16;

        output += noteOutput;

        return true;
    }
};

//-------------------------------------------------------

class NotesTimeUpdater : public NotesIterator
{
public:
    NotesTimeUpdater()
    {
    }

    bool run(Note &note, size_t i, size_t len) override
    {
        updateNoteEnvelope(note, millis());
        return true;
    }
};

//-------------------------------------------------------

class SynthesizerService_CLASS
{

private:
    uint32_t sampleTime = 0;
    NotesTimeUpdater notesTimeUpdater;
    NotesRunner notesRunner;

public:
    SynthesizerService_CLASS()
    {
    }
    virtual ~SynthesizerService_CLASS()
    {
    }

    void begin()
    {
        //wave = new SawtoothWaveGenerator();
        wave = new SinWaveGenerator(100);
    }
    void end()
    {
        delete wave;
    }

    void setWave(WaveType type, size_t resolution)
    {
        STATE_LOCK(lock_wave);
        if (wave != NULL)
        {
            delete wave;
            wave = NULL;
        }

        switch (type)
        {
        case WaveType::SIN:
            wave = new SinWaveGenerator(resolution);
            break;
        case WaveType::SAWTOOTH:
            wave = new SawtoothWaveGenerator();
            break;
        }
        STATE_UNLOCK(lock_wave);
    }

    void writeBuffer(I2S_Frame *buffer, int32_t len)
    {

        byte volume = MidiState.volume();
        // write buffers
        for (int sample = 0; sample < len; sample++)
        {
            MidiState.sampleTime(sampleTime);
            notesRunner.sampleTime = sampleTime;
            notesRunner.output = 0;

            // STATE_LOCK(lock_wave);
            MidiState.notesForeach(&notesRunner);
            // STATE_UNLOCK(lock_wave);

            int32_t totalOutput = notesRunner.output;

            totalOutput = (totalOutput * volume) >> 8;

            // trim:
            totalOutput = fastSigmoid_signed_32_to_16(totalOutput);
            // if (totalOutput > INT16_MAX)
            //     totalOutput = INT16_MAX;
            // else if (totalOutput < INT16_MIN)
            //     totalOutput = INT16_MIN;

            buffer[sample].channel1 = totalOutput;
            buffer[sample].channel2 = totalOutput;

            sampleTime += 1;
        }
    }

    void loop()
    {
        // update envelope sometimes
        static unsigned long lastTime = 0;
        unsigned long now = millis();
        if (now > lastTime + 300 || now < lastTime /*overflow*/)
        {
            MidiState.notesCleanDead();
            lastTime = now;
        }
    }
};

//-----------------------------------------------------------------
// global :
SynthesizerService_CLASS SynthesizerService;
//-----------------------------------------------------------------
