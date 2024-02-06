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

//-------------------------------------------------------
//-------------------------------------------------------

class NotesRunner : public NotesIterator
{
public:
    float output = 0;
    float time = 0;

    float lastEnvelopeUpdateMillis = 0;
    bool requestEnvelopeUpdate = false;

    NotesRunner()
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
        float noteOutput = 0;
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
        float freq = note.freq;
        float angle = (freq)*time;
        angle = PI_2 * (angle - floorf(angle));
        float phase = note.phase;

        // wave (saw tooth for now):
        noteOutput = wave_sawtooth(angle, phase);

        // save angle (for pitch bend changes , need to know the last angle)
        note.angle = angle;

        // envelope:
        noteOutput *= note.currentAmplitude;
        noteOutput *= note.velocityFactor;

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

        // log env graph
        // Serial.printf(">>%d %.2f : ", note.state, note.currentAmplitude);
        // for (int i = 0; i < note.currentAmplitude * 10; i++)
        // {
        //     Serial.print("*");
        // }
        // Serial.println();

        return true;
    }
};

//-------------------------------------------------------

class SynthesizerService_CLASS
{

private:
    int bitrate = 44100;
    float deltaTime = 1.0 / bitrate;
    float time = 0;
    float amplitude = 5000.0; // -32,768 to 32,767

    NotesTimeUpdater notesTimeUpdater;

public:
    SynthesizerService_CLASS()
    {
    }
    virtual ~SynthesizerService_CLASS()
    {
    }

    void begin()
    {
    }

    void writeBuffer(I2S_Frame *buffer, int32_t len)
    {
        NotesRunner runner;

        // write buffers
        for (int sample = 0; sample < len; sample++)
        {
            runner.time = time;
            runner.output = 0;

            MidiState.notesForeach(&runner);
            float output = runner.output;

            output = this->amplitude * output;
            buffer[sample].channel1 = output;
            buffer[sample].channel2 = output;

            time += deltaTime;
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
