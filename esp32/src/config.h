#pragma once

#include <Arduino.h>

const char *ssid = "yak 2.4";
const char *password = "woofwoof";

const uint16_t SERVER_PORT = 80;

const String host_ip = "192.168.1.14";
const int host_port = 3000;
const String host_path = "/log";

const unsigned long UPDATE_ENVELOPE_TIME = 50; // millis