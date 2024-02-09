#pragma once

#include <Arduino.h>

//-------------------------------------------------------

struct GraphPoint
{
    uint16_t x;
    int16_t y;
    uint16_t dx;
    int16_t dy;

    GraphPoint()
    {
        this->x = 0;
        this->y = 0;
        this->dx = 1;
        this->dy = 0;
    }

    GraphPoint(uint16_t x, int16_t y)
    {
        this->x = x;
        this->y = y;
        this->dx = 1;
        this->dy = 0;
    }
};

//-------------------------------------------------------

class WaveInterpolator
{

private:
    size_t numPoints;
    GraphPoint *graph;

public:
    WaveInterpolator(size_t numPoints, int16_t (*waveFunc)(uint16_t x))
    {
        this->numPoints = numPoints;
        this->graph = new GraphPoint[numPoints];

        // prepare graph :
        if (numPoints == 1)
        {
            uint16_t x = 0xFFF;
            int16_t y = waveFunc(x);
            graph[0] = GraphPoint(x, y);
            return;
        }
        //calc x and y
        for (size_t i = 0; i < numPoints; i++)
        {
            uint16_t x = (uint32_t)UINT16_MAX * i / (numPoints - 1);
            int16_t y = waveFunc(x);
            graph[i] = GraphPoint(x, y);
        }
        //precalc dx dy
        for (size_t i = 0; i < numPoints - 1; i++)
        {
            auto a = graph[i];
            auto b = graph[i + 1];
            graph[i].dx = (int16_t)(b.x) - a.x;
            graph[i].dy = b.y - a.y;
        }
    }

    ~WaveInterpolator()
    {
        delete[] this->graph;
    }

    int16_t calc(uint16_t x)
    {
        size_t idx = ((uint32_t)x * (numPoints - 1)) >> 16; // >>16 is div by UINT16_MAX;

        auto a = graph[idx];

        return a.y + (int32_t)(x - a.x) * a.dy / a.dx;
    }
};
