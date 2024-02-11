#pragma once

#include <Arduino.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include "state/midiState.h"
#include "types/midi.h"
#include "consts.h"
#include "logger.h"
#include "utils/utilsSound.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

//-------------------------------------------------------
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Logger.printf("note on %d %d %d\n", channel, pitch, velocity);

    // Envelope envelope(1000, 1000, 0.5, 1000);
    // Envelope envelope(100, 200, 0.8, 300);
    Envelope envelope = MidiState.envelope();
    Note note(pitch, velocity, envelope);
    note.freq = getNoteFrequency(note.pitch, MidiState.pitchBend());
    MidiState.addNote(note);
}

//-------------------------------------------------------
void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Logger.printf("note off %d %d %d\n", channel, pitch, velocity);

    MidiState.releaseNote(pitch);
}

void handleControlChange(byte channel, byte type, byte val)
{
    Logger.printf("ControlChange %d %d %d\n", channel, type, val);

    if (type == midi::MidiControlChangeNumber::ChannelVolume)
    {
        MidiState.volume(val);
    }
    else if (type == midi::MidiControlChangeNumber::ModulationWheel)
    {
    }
}
void handlePitchBend(byte channel, int change)
{
    // change : +/-0x2000 = +/-8192
    // convert to (-2 - +2)
    float val = ((float)change) / 0x1000;
    val = val + MidiState.pitchBendBias();
    MidiState.pitchBend(val);

    // Logger.printf("PitchBend %d %d (%.2f)\n", channel, change, val);
}

void handleError(int8_t err)
{
    Logger.printf("Error %d\n", err);
}
void handleSystemReset()
{
    Logger.printf("SystemReset\n");
}

void handleAfterTouchPoly(byte channel, byte note, byte velocity) { Logger.printf("AfterTouchPoly\n"); }
void handleProgramChange(byte channel, byte) { Logger.printf("ProgramChange\n"); }
void handleAfterTouchChannel(byte channel, byte) { Logger.printf("AfterTouchChannel\n"); }
void handleSystemExclusive(byte *array, unsigned size) { Logger.printf("SystemExclusive\n"); }
void handleTimeCodeQuarterFrame(byte data) { Logger.printf("TimeCodeQuarterFrame\n"); }
void handleSongPosition(unsigned beats) { Logger.printf("SongPosition\n"); }
void handleSongSelect(byte songnumber) { Logger.printf("SongSelect\n"); }
void handleTuneRequest() { Logger.printf("TuneRequest\n"); }
void handleClock() { Logger.printf("Clock\n"); }
void handleStart() { Logger.printf("Start\n"); }
void handleTick() { Logger.printf("Tick\n"); }
void handleContinue() { Logger.printf("Continue\n"); }
void handleStop() { Logger.printf("Stop\n"); }
void handleActiveSensing() { Logger.printf("ActiveSensing\n"); }

class MidiServiceClass
{
public:
    MidiServiceClass()
    {
    }

    void begin()
    {
        // register events :
        MIDI.setHandleNoteOn(handleNoteOn);
        MIDI.setHandleNoteOff(handleNoteOff);

        MIDI.setHandleControlChange(handleControlChange); // ([](byte channel, byte, byte) -> void { Serial.println("ControlChange"); });
        MIDI.setHandlePitchBend(handlePitchBend);         // ([](byte channel, int) -> void { Serial.println("PitchBend"); });
        MIDI.setHandleError(handleError);                 // ([](int8_t err) -> void { Serial.println("Error"); });
        MIDI.setHandleSystemReset(handleSystemReset);     // ([]() -> void { Serial.println("SystemReset"); });

        MIDI.setHandleAfterTouchPoly(handleAfterTouchPoly);             // ([](byte channel, byte note, byte velocity) -> void { Serial.println("AfterTouchPoly"); });
        MIDI.setHandleProgramChange(handleProgramChange);               // ([](byte channel, byte) -> void { Serial.println("ProgramChange"); });
        MIDI.setHandleAfterTouchChannel(handleAfterTouchChannel);       // ([](byte channel, byte) -> void { Serial.println("AfterTouchChannel"); });
        MIDI.setHandleSystemExclusive(handleSystemExclusive);           // ([](byte *array, unsigned size) -> void { Serial.println("SystemExclusive"); });
        MIDI.setHandleTimeCodeQuarterFrame(handleTimeCodeQuarterFrame); // ([](byte data) -> void { Serial.println("TimeCodeQuarterFrame"); });
        MIDI.setHandleSongPosition(handleSongPosition);                 // ([](unsigned beats) -> void { Serial.println("SongPosition"); });
        MIDI.setHandleSongSelect(handleSongSelect);                     // ([](byte songnumber) -> void { Serial.println("SongSelect"); });
        MIDI.setHandleTuneRequest(handleTuneRequest);                   // ([]() -> void { Serial.println("TuneRequest"); });
        MIDI.setHandleClock(handleClock);                               // ([]() -> void { Serial.println("Clock"); });
        MIDI.setHandleStart(handleStart);                               // ([]() -> void { Serial.println("Start"); });
        MIDI.setHandleTick(handleTick);                                 // ([]() -> void { Serial.println("Tick"); });
        MIDI.setHandleContinue(handleContinue);                         // ([]() -> void { Serial.println("Continue"); });
        MIDI.setHandleStop(handleStop);                                 // ([]() -> void { Serial.println("Stop"); });
        // MIDI.setHandleActiveSensing(handleActiveSensing);               // ([]() -> void { Serial.println("ActiveSensing"); });

        // ------
        MIDI.begin(MIDI_CHANNEL_OMNI);
        Logger.println("midi init done");
    }

    void loop()
    {
        MIDI.read();
    }
};

MidiServiceClass MidiService;
