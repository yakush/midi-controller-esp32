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

struct SimpleNote
{
    FREQ_T freq;
    FREQ_T phase;
    EnvelopeState state;
    uint16_t currentAmplitude;
};

//-------------------------------------------------------
class NotesTimeUpdater : public NotesIterator
{
    unsigned long now = 0;

public:
    NotesTimeUpdater()
    {
    }

    bool before(size_t len) override
    {
        this->now = millis();
        return true;
    }

    bool run(Note &note, size_t i, size_t len) override
    {
        updateNoteEnvelope(note, now);
        return true;
    }
};

//-------------------------------------------------------

class SynthesizerService_CLASS
{

private:
    uint32_t sampleTime = 0;
    NotesTimeUpdater notesTimeUpdater;
    SimpleNote notes[MAX_NOTES];
    size_t notesLen = 0;
    float pitchBend;

public:
    SynthesizerService_CLASS()
    {
    }
    virtual ~SynthesizerService_CLASS()
    {
    }

    void begin()
    {
        pitchBend = MidiState.pitchBend();

        // wave = new SawtoothWaveGenerator();
        wave = new SinWaveGenerator(1000);
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
        // copy to local array
        // - will also run updateNoteEnvelope() if never ran on note
        // - will also ignore dead notes
        MidiState.lock();
        auto origNotes = MidiState.notes();
        auto origNotesLen = MidiState.notesLen();
        auto volume = MidiState.volume();
        auto newPitchBend = MidiState.pitchBend();

        notesLen = 0;

        for (size_t i = 0; i < origNotesLen; i++)
        {
            // skip dead notes
            if (origNotes[i].state == EnvelopeState::DEAD)
                continue;

            // update brand new notes
            if (origNotes[i].state == EnvelopeState::PRESSED)
                updateNoteEnvelope(origNotes[i], millis());

            // update freq from pitch band:
            if (newPitchBend != pitchBend)
            {
                auto oldFreq = origNotes[i].freq;
                auto oldPhase = origNotes[i].phase;
                auto newFreq = getNoteFrequency(origNotes[i].pitch, newPitchBend);
                int64_t newPhase = (int64_t)oldPhase + sampleTime * ((int64_t)oldFreq - newFreq);

                newPhase = newPhase % FREQ_MAX;
                if (newPhase < 0)
                    newPhase += FREQ_MAX;

                origNotes[i].freq = newFreq;
                origNotes[i].phase = newPhase;
            }

            // copy to local array
            this->notes[notesLen].freq = origNotes[i].freq;
            this->notes[notesLen].phase = origNotes[i].phase;
            this->notes[notesLen].state = origNotes[i].state;
            this->notes[notesLen].currentAmplitude = origNotes[i].currentAmplitude;

            notesLen++;
        }
        pitchBend = newPitchBend;
        MidiState.unlock();

        // MidiState.notesForeach(&copyFromSource);
        // notesLen = copyFromSource.len;
        // auto volume = MidiState.volume();

        // go
        for (int sample = 0; sample < len; sample++)
        {
            int32_t totalOutput = 0;
            for (size_t i = 0; i < notesLen; i++)
            {
                // build wave for note
                auto note = notes[i];
                int32_t noteOutput = 0;

                // angle is (0..WAVE_PI_2) as in range of (0..pi)
                FREQ_T angle = calcWaveAngleFromTime(sampleTime, note.freq, note.phase);
                noteOutput = wave->calc(angle);
                noteOutput = (noteOutput * note.currentAmplitude) >> 16;
                totalOutput += noteOutput;
            }

            // sum and trim:
            totalOutput = (totalOutput * volume) >> 8;
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
        if (now > lastTime + UPDATE_ENVELOPE_TIME || now < lastTime /*overflow*/)
        {
            MidiState.notesForeach(&notesTimeUpdater);
            MidiState.notesCleanDead();
            lastTime = now;
        }
    }
};

//-----------------------------------------------------------------
// global :
SynthesizerService_CLASS SynthesizerService;
//-----------------------------------------------------------------
