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

void setup()
{
  Serial.begin(115200);
  Logger.add(&Serial);

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

  if (!SpeakerService.begin(
          0,    // i2s port (0 or 1)
          26,   // BCK pin
          25,   // LRCK pin
          22,   // DATA pin
          44100 // play freq
          ))
  {
    Serial.println("MAX i2s driver initialization Failed");
    // return;
  };
}

void loop()
{
  ServerService.loop();
  MidiService.loop();
  SynthesizerService.loop();

  { // log i2s_writeTime
    static unsigned long last = millis();
    if (millis() > last + 1000)
    {
      Logger.printf("time %u micros\n", AppState.i2s_writeTime());
      last = millis();
    }
  }
  // static unsigned long last = millis();
  // static unsigned long counter = 1;
  // if (millis() > last + 10000)
  // {
  //   // Logger.println("time:");
  //   // ws.printfAll("hi %d", counter);
  //   Logger.printf("hi %ul\n", counter);
  //   last = millis();
  //   counter++;
  // }
}
