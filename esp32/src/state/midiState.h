#pragma once
#include <Arduino.h>
#include <mutex>
#include <ArduinoJson.h>
#include "state.h"
#include "types/midi.h"
#include "utils/utilsSound.h"
#include "waveGenerators.h"

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
    byte _volume = INITIAL_VOLUME_NORMALIZED * 0xFF;
    /** range (-2 .. +2) semitones */
    float _pitchBend = 0;
    float _pitchBendBias = INITIAL_PITCH_BEND_BIAS;
    std::vector<Note> _notesPlaying;

    /** last played sample-time */
    uint32_t _sampleTime = 0;

    Envelope _envelope = Envelope(100, 200, 0.8, 300);

public:
    void writeToJson(JsonVariant &json)
    {
        json["volume"] = this->volumeNormalized();
        json["pitchBend"] = this->pitchBend();
        json["pitchBendBias"] = this->pitchBendBias();

        auto envelopeJson = json.createNestedObject("envelope");
        auto envelope = this->envelope();
        envelopeJson["attack"] = envelope.attack;
        envelopeJson["decay"] = envelope.decay;
        envelopeJson["sustain"] = envelope.sustainNormalized();
        envelopeJson["release"] = envelope.release;
    }

    void readFromJson(JsonVariant &json, bool log = false)
    {
        auto jsonObj = json.as<JsonObject>();

        if (jsonObj.containsKey("volume"))
        {
            float volume = jsonObj["volume"];
            this->volumeNormalized(volume);
            if (log)
                Logger.printf("got volume: %.2f\n", volume);
        }
        if (jsonObj.containsKey("pitchBend"))
        {
            float pitchBend = jsonObj["pitchBend"];
            this->pitchBend(pitchBend);
            if (log)
                Logger.printf("got pitchBend: %.2f\n", pitchBend);
        }
        if (jsonObj.containsKey("pitchBendBias"))
        {
            float pitchBendBias = jsonObj["pitchBendBias"];
            this->pitchBendBias(pitchBendBias);
            if (log)
                Logger.printf("got pitchBendBias: %.2f\n", pitchBendBias);
        }
        if (jsonObj.containsKey("envelope"))
        {
            JsonObject envelopeJson = jsonObj["envelope"];
            Envelope envelope = this->envelope();
            if (envelopeJson.containsKey("attack"))
            {
                uint16_t attack = envelopeJson["attack"];
                envelope.attack = attack;
            }
            if (envelopeJson.containsKey("decay"))
            {
                uint16_t decay = envelopeJson["decay"];
                envelope.decay = decay;
            }
            if (envelopeJson.containsKey("sustain"))
            {
                float sustain = envelopeJson["sustain"];
                envelope.sustainNormalized(sustain);
            }
            if (envelopeJson.containsKey("release"))
            {
                uint16_t release = envelopeJson["release"];
                envelope.release = release;
            }
            this->envelope(envelope);
            if (log)
            {
                Logger.printf("got envelope: [%d, %d, %.2f, %d]\n", envelope.attack, envelope.decay, envelope.sustainNormalized(), envelope.release);
            }
        }
    }

    byte volume() { return _volume; }
    void volume(byte value)
    {
        STATE_LOCK(_mutex);
        _volume = value;
        STATE_UNLOCK(_mutex);
    }
    float volumeNormalized() { return (float)_volume / 0xFF; }
    void volumeNormalized(float normalized)
    {
        volume((byte)(normalized * 0xFF));
    }

    Envelope envelope() { return _envelope; }
    void envelope(Envelope value)
    {
        STATE_LOCK(_mutex);
        _envelope = value;
        STATE_UNLOCK(_mutex);
    }

    uint32_t sampleTime() { return _sampleTime; }
    void sampleTime(uint32_t value)
    {
        STATE_LOCK(_mutex);
        _sampleTime = value;
        STATE_UNLOCK(_mutex);
    }

    float pitchBend() { return _pitchBend; }
    void pitchBend(float value)
    {
        STATE_LOCK(_mutex);
        _pitchBend = value;
        STATE_UNLOCK(_mutex);
    }

    float pitchBendBias() { return _pitchBendBias; }
    void pitchBendBias(float value)
    {
        STATE_LOCK(_mutex);
        _pitchBendBias = value;
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
        bool died = false;

        STATE_LOCK(_mutex);
        for (size_t i = 0; i < _notesPlaying.size(); i++)
        {
            if (_notesPlaying[i].pitch == pitch)
            {
                updateNoteRelease(_notesPlaying[i], millis());
                died = _notesPlaying[i].state == EnvelopeState::DEAD;
                break;
            }
        }
        STATE_UNLOCK(_mutex);

        if (died)
        {
            notesCleanDead();
        }
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