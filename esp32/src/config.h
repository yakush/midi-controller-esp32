#pragma once

#include <Arduino.h>

const char *ssid = "yak 2.4";
const char *password = "woofwoof";

const uint16_t SERVER_PORT = 80;

const String host_ip = "192.168.1.14";
const int host_port = 3000;
const String host_path = "/log";

#define INITIAL_PITCH_BEND_BIAS (-0.84)
#define INITIAL_VOLUME_NORMALIZED (0.3)

#define SAMPLE_RATE (44100)
#define WAVE_SAMPLE_RATE (0xffff)
#define SHIFT_WAVE_SAMPLE_RATE (16)

const unsigned long UPDATE_ENVELOPE_TIME = 20; // millis

// isp buffer elements sized
#define FREQ_T uint16_t
#define FREQ_MAX UINT16_MAX

//#define SHIFT_FRAME_CHANNEL (16)
#define FRAME_CHANNEL_T int16_t
//#define FRAME_CHANNEL_MAX UINT16_MAX
#define FRAME_CHANNEL_DOUBLE_T int32_t
//#define FRAME_CHANNEL_DOUBLE_MAX UINT32_MAX