#pragma once
#include <Arduino.h>
#include <mutex>
#include "state.h"
#include "types/midi.h"

class NotesIterator
{
public:
    /**
     * @return continue the loop?
     */
    virtual bool before(size_t len) { return true; }

    /**
     * @return continue the loop?
     */
    virtual bool run(Note &note, size_t i, size_t len) = 0;

    virtual void after(size_t len) {}
};

//-------------------------------------------------------

class MidiState_Class
{
private:
    STATE_LOCK_DEFINE(_mutex);
    byte _volume = 30;
    /** range (-2 .. +2) semitones */
    float _pitchBend = 0;
    std::vector<Note> _notesPlaying;

public:
    byte volume() { return _volume; }
    void volume(byte value)
    {
        STATE_LOCK(_mutex);
        _volume = value;
        STATE_UNLOCK(_mutex);
    }

    float pitchBend() { return _pitchBend; }
    void pitchBend(float value)
    {
        STATE_LOCK(_mutex);
        _pitchBend = value;
        STATE_UNLOCK(_mutex);
    }

    void getNotesPlaying(std::vector<Note> &output)
    {
        STATE_LOCK(_mutex);
        output.assign(_notesPlaying.begin(), _notesPlaying.end());
        STATE_UNLOCK(_mutex);
    }

    void addNote(Note &note)
    {
        STATE_LOCK(_mutex);
        removeNote(note.pitch);
        _notesPlaying.push_back(note);

        logNotes();

        STATE_UNLOCK(_mutex);
    }

    void removeNote(byte pitch)
    {
        STATE_LOCK(_mutex);
        size_t len = _notesPlaying.size();

        _notesPlaying.erase(
            std::remove_if(_notesPlaying.begin(), _notesPlaying.end(),
                           [pitch](const Note &note)
                           { return note.pitch == pitch; }),
            _notesPlaying.end());

        if (len != _notesPlaying.size())
        {
            logNotes();
        }
        STATE_UNLOCK(_mutex);
    }

    void releaseNote(byte pitch)
    {
        STATE_LOCK(_mutex);
        for (size_t i = 0; i < _notesPlaying.size(); i++)
        {
            if (_notesPlaying[i].pitch == pitch)
            {
                if (
                    _notesPlaying[i].state != EnvelopeState::DEAD &&
                    _notesPlaying[i].state != EnvelopeState::RELEASE)
                {
                    _notesPlaying[i].state = EnvelopeState::RELEASE;
                    _notesPlaying[i].stateStartTime = millis();
                    _notesPlaying[i].startReleaseAmplitude = _notesPlaying[i].currentAmplitude;

                    // log the change
                    logNotes();
                }
                break;
            }
        }
        STATE_UNLOCK(_mutex);
    }

    void removeAllNotes()
    {
        STATE_LOCK(_mutex);
        size_t len = _notesPlaying.size();

        _notesPlaying.clear();

        if (len != _notesPlaying.size())
        {
            logNotes();
        }
        STATE_UNLOCK(_mutex);
    }

    void notesForeach(NotesIterator *iterator)
    {
        STATE_LOCK(_mutex);
        size_t len = _notesPlaying.size();
        bool okToContinue = iterator->before(len);
        for (size_t i = 0; i < len; i++)
        {
            okToContinue = iterator->run(_notesPlaying[i], i, len);
            if (!okToContinue)
            {
                break;
            }
        }
        iterator->after(len);
        STATE_UNLOCK(_mutex);
    }

    void notesCleanDead()
    {
        STATE_LOCK(_mutex);
        size_t len = _notesPlaying.size();
        _notesPlaying.erase(
            std::remove_if(_notesPlaying.begin(), _notesPlaying.end(),
                           [](const Note &note)
                           { return note.state == EnvelopeState::DEAD; }),
            _notesPlaying.end());

        if (len != _notesPlaying.size())
        {
            logNotes();
        }
        STATE_UNLOCK(_mutex);
    }

    void logNotes()
    {
        String out = "> notes: ";

        STATE_LOCK(_mutex);
        for (size_t i = 0; i < _notesPlaying.size(); i++)
        {
            auto pitch = _notesPlaying[i].pitch;
            auto freq = _notesPlaying[i].freq;
            auto velocity = _notesPlaying[i].velocity;
            auto velocityFactor = _notesPlaying[i].velocityFactor;
            // Logger.printf("[%d %.2f]", pitch, freq);
            out = out + "[" + pitch + " " + freq + "hz v: " + velocity + "=" + velocityFactor + "]";
        }
        STATE_UNLOCK(_mutex);
        out = out + "\n";
        Logger.print(out);
    }
};

MidiState_Class MidiState;