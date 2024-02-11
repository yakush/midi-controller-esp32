#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "serverService.h"
#include "midiService.h"
#include "speakerService.h"
#include "synthesizerService.h"
#include "webLogger.h"
#include "logger.h"
#include "state/midiState.h"
#include "state/appState.h"

#include "utils/utilsSound.h"
#include "waveGenerators.h"

void test_waves()
{
  SawtoothWaveGenerator sawtooth;
  logGraphChannelValue(" > ", 00000, 10);
  logGraphChannelValue(" > ", 10000, 10);
  logGraphChannelValue(" > ", 20000, 10);
  logGraphChannelValue(" > ", 30000, 10);
  logGraphChannelValue(" > ", 40000, 10);
  logGraphChannelValue(" > ", 50000, 10);
  logGraphChannelValue(" > ", 60000, 10);
  Serial.println();

  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (0.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (1.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (2.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (3.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (4.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (5.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (6.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (7.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (8.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (9.0 / 10)), 15, true);
  logGraphChannelValue("wave i16: ", sawtooth.calc(WAVE_PI_2 * (10.0 / 10)), 15, true);
  Serial.println();
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (0.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (1.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (2.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (3.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (4.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (5.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (6.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (7.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (8.0 / 10))), 15, true);
  logGraphChannelValue("wave u16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (9.0 / 10))), 15, true);
  logGraphChannelValue("wave i16: ", (uint16_t)(0x7fff + sawtooth.calc(WAVE_PI_2 * (10.0 / 10))), 15, true);

  static const size_t NUM = 1000;
  I2S_Frame buffer[NUM];

  // clip
  Serial.printf("CLIP TEST\n");
  for (size_t i = 0; i < NUM; i += 1)
  {
    // int32_t val = ((int32_t)i - NUM / 2) * 10 * 0x7FFF / NUM;
    int32_t val = ((int32_t)i * 0xFFFF / NUM) - 0x7FFF;
    val *= 8;

    val = std::min(val, INT16_MAX);
    val = std::max(val, INT16_MIN);

    buffer[i].channel1 = buffer[i].channel2 = val;
  }
  for (size_t i = 0; i < NUM; i += NUM / 25)
  {
    logGraphChannelValue(">", buffer[i].channel1, 50, true);
  }
  // sigmoid
  Serial.printf("SIGMOID TEST\n");
  for (size_t i = 0; i < NUM; i += 1)
  {
    int32_t val = ((int32_t)i * 0xFFFF / NUM) - 0x7FFF;
    val *= 8;

    val = fastSigmoid_signed_32_to_16(val);

    buffer[i].channel1 = buffer[i].channel2 = val;
  }
  for (size_t i = 0; i < NUM; i += NUM / 25)
  {
    logGraphChannelValue(">", buffer[i].channel1, 50, true);
  }

  Envelope envelope(0, 300, 0.8, 200);

  // wave note:
  {
    auto oldVolume = MidiState.volume();
    MidiState.volumeNormalized(0.99);
    Serial.printf("WAVE TEST\n");
    Note note(60, 125, envelope);
    MidiState.addNote(note);
    SynthesizerService.writeBuffer(buffer, NUM);
    for (size_t i = 0; i < NUM; i += NUM / 50)
    {
      logGraphChannelValue(">", buffer[i].channel1, 50, true);
    }
    MidiState.volume(oldVolume);
  }

  // -- pitch bend phase adjust test
  {
    Serial.printf("phase adjust test\n");
    MidiState.removeAllNotes();
    auto oldVolume = MidiState.volume();
    MidiState.volumeNormalized(0.99);
    Note note(45, 125, envelope);
    MidiState.addNote(note);

    MidiState.pitchBend(0);
    SynthesizerService.writeBuffer(buffer, NUM);
    Serial.printf("---f %u p %u\n", MidiState.notes()[0].freq, MidiState.notes()[0].phase);
    for (size_t i = 0; i < NUM; i += NUM / 50)
      logGraphChannelValue(">", buffer[i].channel1, 50, true);

    MidiState.pitchBend(1);
    SynthesizerService.writeBuffer(buffer, NUM);
    Serial.printf("---f %u p %u\n", MidiState.notes()[0].freq, MidiState.notes()[0].phase);
    for (size_t i = 0; i < NUM; i += NUM / 50)
      logGraphChannelValue(">", buffer[i].channel1, 50, true);

    MidiState.pitchBend(-1);
    SynthesizerService.writeBuffer(buffer, NUM);
    Serial.printf("---f %u p %u\n", MidiState.notes()[0].freq, MidiState.notes()[0].phase);
    for (size_t i = 0; i < NUM; i += NUM / 50)
      logGraphChannelValue(">", buffer[i].channel1, 50, true);

    MidiState.pitchBend(2);
    SynthesizerService.writeBuffer(buffer, NUM);
    Serial.printf("---f %u p %u\n", MidiState.notes()[0].freq, MidiState.notes()[0].phase);
    for (size_t i = 0; i < NUM; i += NUM / 50)
      logGraphChannelValue(">", buffer[i].channel1, 50, true);

    MidiState.pitchBend(0);
    MidiState.volume(oldVolume);
  }

  {
    Note note(60, 125, envelope);
    Serial.printf("vfactor: %u\n", note.velocityFactor);
  }

  // -----
  // time test
  MidiState.removeAllNotes();

  uint64_t start;
  uint64_t end;
  int TESTS = 1000;

  {
    start = esp_timer_get_time();
    for (size_t i = 0; i < TESTS; i++)
    {
      SynthesizerService.writeBuffer(buffer, 64);
    }
    end = esp_timer_get_time();
    Logger.printf("0 note time %u micros\n", (end - start) / TESTS);
  }
  // -----
  {
    Note note(70, 60, envelope);
    MidiState.addNote(note);

    start = esp_timer_get_time();
    for (size_t i = 0; i < TESTS; i++)
    {
      SynthesizerService.writeBuffer(buffer, 64);
    }
    end = esp_timer_get_time();
    Logger.printf("1 note time %u micros\n", (end - start) / TESTS);
  }
  // -----
  {
    Note note(80, 60, envelope);
    MidiState.addNote(note);

    start = esp_timer_get_time();
    for (size_t i = 0; i < TESTS; i++)
    {
      SynthesizerService.writeBuffer(buffer, 64);
    }
    end = esp_timer_get_time();
    Logger.printf("2 note time %u micros\n", (end - start) / TESTS);
  }
  // -----
  {
    Note note(90, 60, envelope);
    MidiState.addNote(note);

    start = esp_timer_get_time();
    for (size_t i = 0; i < TESTS; i++)
    {
      SynthesizerService.writeBuffer(buffer, 64);
    }
    end = esp_timer_get_time();
    Logger.printf("3 note time %u micros\n", (end - start) / TESTS);
  }
  // -----
  {

    Note note(100, 60, envelope);
    MidiState.addNote(note);

    start = esp_timer_get_time();
    for (size_t i = 0; i < TESTS; i++)
    {
      SynthesizerService.writeBuffer(buffer, 64);
    }
    end = esp_timer_get_time();
    Logger.printf("4 note time %u micros\n", (end - start) / TESTS);
  }
  // -----
  {

    // change pitch bend
    MidiState.pitchBend(0.2);

    start = esp_timer_get_time();
    for (size_t i = 0; i < TESTS; i++)
    {
      SynthesizerService.writeBuffer(buffer, 64);
    }
    end = esp_timer_get_time();
    Logger.printf("4 note + pitchBend time %u micros\n", (end - start) / TESTS);
  }
  // -----
  MidiState.removeAllNotes();
}