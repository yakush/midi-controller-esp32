#pragma once

#include "config.h"
#include <Arduino.h>
#include <vector>

class LoggerClass : public Print
{
private:
    std::vector<Print *> loggers;

public:
    LoggerClass()
    {
    }

    void add(Print *logger)
    {
        loggers.push_back(logger);
    }

    void remove(Print *logger)
    {
        for (size_t i = 0; i < loggers.size(); i++)
        {
            if (loggers[i] == logger)
            {
                loggers.erase(loggers.begin() + i);
                break;
            }
        }
    }

    size_t write(uint8_t x)
    {
        for (size_t i = 0; i < loggers.size(); i++)
        {
            loggers[i]->write(x);
        }
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size)
    {
        for (size_t i = 0; i < loggers.size(); i++)
        {
            loggers[i]->write(buffer, size);
        }
        return size;
    }
};

LoggerClass Logger;
