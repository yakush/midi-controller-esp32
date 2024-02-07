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
  logGraphChannelValue(" > ", 00000, 10);
  logGraphChannelValue(" > ", 10000, 10);
  logGraphChannelValue(" > ", 20000, 10);
  logGraphChannelValue(" > ", 30000, 10);
  logGraphChannelValue(" > ", 40000, 10);
  logGraphChannelValue(" > ", 50000, 10);
  logGraphChannelValue(" > ", 60000, 10);
  Serial.println();

  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (0.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (1.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (2.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (3.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (4.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (5.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (6.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (7.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (8.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (9.0 / 10), 0), 15);
  logGraphChannelValue("wave: ", wave_sawtooth(WAVE_PI_2 * (10.0 / 10), 0), 15);

  static const size_t NUM = 1000;
  Envelope envelope(100, 300, 0.8, 200);
  Note note(60, 60, envelope);
  MidiState.addNote(note);
  I2S_Frame buffer[NUM];
  SynthesizerService.writeBuffer(buffer, NUM);
  for (size_t i = 0; i < NUM; i += NUM / 50)
  {
    logGraphChannelValue(">", buffer[i].channel1, 50);
  }
  Serial.printf("vfactor: %u\n", note.velocityFactor);

  // -----
  // time test
  MidiState.removeAllNotes();

  uint64_t start;
  uint64_t end;
  int TESTS = 1000;

  start = esp_timer_get_time();
  for (size_t i = 0; i < TESTS; i++)
  {
    SynthesizerService.writeBuffer(buffer, 64);
  }
  end = esp_timer_get_time();
  Logger.printf("0 note time %u micros\n", (end - start) / TESTS);
  // -----
  {
    Envelope envelope(100, 300, 0.8, 200);
    Note note(70, 60, envelope);
    MidiState.addNote(note);
  }

  start = esp_timer_get_time();
  for (size_t i = 0; i < TESTS; i++)
  {
    SynthesizerService.writeBuffer(buffer, 64);
  }
  end = esp_timer_get_time();
  Logger.printf("1 note time %u micros\n", (end - start) / TESTS);
  // -----
  {
    Envelope envelope(100, 300, 0.8, 200);
    Note note(80, 60, envelope);
    MidiState.addNote(note);
  }

  start = esp_timer_get_time();
  for (size_t i = 0; i < TESTS; i++)
  {
    SynthesizerService.writeBuffer(buffer, 64);
  }
  end = esp_timer_get_time();
  Logger.printf("2 note time %u micros\n", (end - start) / TESTS);
  // -----
  {
    Envelope envelope(100, 300, 0.8, 200);
    Note note(90, 60, envelope);
    MidiState.addNote(note);
  }

  start = esp_timer_get_time();
  for (size_t i = 0; i < TESTS; i++)
  {
    SynthesizerService.writeBuffer(buffer, 64);
  }
  end = esp_timer_get_time();
  Logger.printf("3 note time %u micros\n", (end - start) / TESTS);
  // -----
  {
    Envelope envelope(100, 300, 0.8, 200);
    Note note(100, 60, envelope);
    MidiState.addNote(note);
  }

  start = esp_timer_get_time();
  for (size_t i = 0; i < TESTS; i++)
  {
    SynthesizerService.writeBuffer(buffer, 64);
  }
  end = esp_timer_get_time();
  Logger.printf("4 note time %u micros\n", (end - start) / TESTS);
  // -----
}