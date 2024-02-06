#pragma once
#include <Arduino.h>
#include "serverService.h"

static size_t webLog_print(const uint8_t *payload, size_t size)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        ServerService.ws().textAll((char *)payload, size);
    }
    return size;
}

class WebLoggerClass : public Print
{

public:
    WebLoggerClass() {}

    void begin()
    {
    }
    void end()
    {
    }

    size_t write(uint8_t x)
    {
        uint8_t buff[1];
        buff[0] = x;
        return webLog_print(buff, 1);
    }

    size_t write(const uint8_t *buffer, size_t size)
    {
        return webLog_print(buffer, size);
    }
};

WebLoggerClass WebLogger;
