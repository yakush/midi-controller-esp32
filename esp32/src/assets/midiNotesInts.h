#pragma once
#include <Arduino.h>
#include "config.h"

const FREQ_T MIDI_NOTES[]{
    8,     // 8.18
    9,     // 8.66
    9,     // 9.18
    10,    // 9.72
    10,    // 10.3
    11,    // 10.91
    12,    // 11.56
    12,    // 12.25
    13,    // 12.98
    14,    // 13.75
    15,    // 14.57
    15,    // 15.43
    16,    // 16.35
    17,    // 17.32
    18,    // 18.35
    19,    // 19.45
    21,    // 20.6
    22,    // 21.83
    23,    // 23.12
    25,    // 24.5
    26,    // 25.96
    28,    // 27.5
    29,    // 29.14
    31,    // 30.87
    33,    // 32.7
    35,    // 34.65
    37,    // 36.71
    39,    // 38.89
    41,    // 41.2
    44,    // 43.65
    46,    // 46.25
    49,    // 49
    52,    // 51.91
    55,    // 55
    58,    // 58.27
    62,    // 61.74
    65,    // 65.41
    69,    // 69.3
    73,    // 73.42
    78,    // 77.78
    82,    // 82.41
    87,    // 87.31
    93,    // 92.5
    98,    // 98
    104,   // 103.83
    110,   // 110
    117,   // 116.54
    123,   // 123.47
    131,   // 130.81
    139,   // 138.59
    147,   // 146.83
    156,   // 155.56
    165,   // 164.81
    175,   // 174.61
    185,   // 185
    196,   // 196
    208,   // 207.65
    220,   // 220
    233,   // 233.08
    247,   // 246.94
    262,   // 261.63
    277,   // 277.18
    294,   // 293.66
    311,   // 311.13
    330,   // 329.63
    349,   // 349.23
    370,   // 369.99
    392,   // 392
    415,   // 415.3
    440,   // 440
    466,   // 466.16
    494,   // 493.88
    523,   // 523.25
    554,   // 554.37
    587,   // 587.33
    622,   // 622.25
    659,   // 659.26
    698,   // 698.46
    740,   // 739.99
    784,   // 783.99
    831,   // 830.61
    880,   // 880
    932,   // 932.33
    988,   // 987.77
    1047,  // 1046.5
    1109,  // 1108.73
    1175,  // 1174.66
    1245,  // 1244.51
    1319,  // 1318.51
    1397,  // 1396.91
    1480,  // 1479.98
    1568,  // 1567.98
    1661,  // 1661.22
    1760,  // 1760
    1865,  // 1864.66
    1976,  // 1975.53
    2093,  // 2093
    2217,  // 2217.46
    2349,  // 2349.32
    2489,  // 2489.02
    2637,  // 2637.02
    2794,  // 2793.83
    2960,  // 2959.96
    3136,  // 3135.96
    3322,  // 3322.44
    3520,  // 3520
    3729,  // 3729.31
    3951,  // 3951.07
    4186,  // 4186.01
    4435,  // 4434.92
    4699,  // 4698.64
    4978,  // 4978.03
    5274,  // 5274.04
    5588,  // 5587.65
    5920,  // 5919.91
    6272,  // 6271.93
    6645,  // 6644.88
    7040,  // 7040
    7459,  // 7458.62
    7902,  // 7902.13
    8372,  // 8372.02
    8870,  // 8869.84
    9397,  // 9397.27
    9956,  // 9956.06
    10548, // 10548.08
    11175, // 11175.3
    11840, // 11839.82
    12544, // 12543.85
};