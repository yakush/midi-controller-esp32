#pragma once

#include <Arduino.h>

const char *ssid = "yak 2.4";
const char *password = "woofwoof";

const uint16_t SERVER_PORT = 80;

const String host_ip = "192.168.1.14";
const int host_port = 3000;
const String host_path = "/log";

#define SAMPLE_RATE (44100)
#define WAVE_SAMPLE_RATE (0xffff)
#define SHIFT_WAVE_SAMPLE_RATE (16)

const unsigned long UPDATE_ENVELOPE_TIME = 50; // millis

// isp buffer elements sized
#define FREQ_T uint16_t
#define SHIFT_FRAME_CHANNEL (16)
#define FRAME_CHANNEL_T uint16_t
#define FRAME_CHANNEL_MAX UINT16_MAX
#define FRAME_CHANNEL_DOUBLE_T uint32_t
#define FRAME_CHANNEL_DOUBLE_MAX UINT32_MAX