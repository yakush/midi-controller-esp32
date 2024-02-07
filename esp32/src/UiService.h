#pragma once

#include <Arduino.h>
#include "state/appState.h"

#define PIN_LED_READY (33)

class UiService_CLASS
{

private:
public:
    UiService_CLASS() {}
    virtual ~UiService_CLASS() {}

    void begin()
    {
        pinMode(PIN_LED_READY, OUTPUT);

        // first time
        this->loop();
    }
    void loop()
    {
        digitalWrite(PIN_LED_READY, AppState.isReady());
    }
};

//-----------------------------------------------------------------
// global :
UiService_CLASS UiService;
//-----------------------------------------------------------------
