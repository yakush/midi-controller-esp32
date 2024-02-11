#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "serverService.h"
#include "midiService.h"
#include "speakerService.h"
#include "synthesizerService.h"
#include "UiService.h"
#include "webLogger.h"
#include "logger.h"
#include "state/midiState.h"
#include "state/appState.h"

#include "utils/utilsSound.h"
#include "waveGenerators.h"
#include "../test/test_waves.h"

void setup()
{
  Serial.begin(115200);
  Logger.add(&Serial);

  UiService.begin();

  // init WIFI:
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  ServerService.begin();

  WebLogger.begin();
  Logger.add(&WebLogger);

  MidiService.begin();
  SynthesizerService.begin();

  test_waves();

  if (!SpeakerService.begin(
          0,          // i2s port (0 or 1)
          26,         // BCK pin
          25,         // LRCK pin
          22,         // DATA pin
          SAMPLE_RATE // play freq
          ))
  {
    Serial.println("MAX i2s driver initialization Failed");
    // return;
  };

  AppState.ready(true);
}
//-------------------------------------------------------

class EnvLogger : public NotesIterator
{
public:
  EnvLogger()
  {
  }

  bool run(Note &note, size_t i, size_t len) override
  {
    Serial.printf("%d [%d] ", note.pitch, note.state);
    logGraphChannelValue(": ", note.currentAmplitude, 20);
    return true;
  }
};
//-------------------------------------------------------

void loop()
{
  ServerService.loop();
  MidiService.loop();
  SynthesizerService.loop();
  UiService.loop();

  // { // log envelope graph
  //   static unsigned long last = millis();
  //   if (millis() > last + 50)
  //   {
  //     EnvLogger logger;
  //     MidiState.notesForeach(&logger);
  //     last = millis();
  //   }
  // }

  { // log i2s_writeTime
    static unsigned long last = millis();
    if (millis() > last + 3000)
    {
      Logger.printf("time %u micros\n", AppState.i2s_writeTime());
      last = millis();
    }
  }
}
