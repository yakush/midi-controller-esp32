#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
// #include <Arduino_JSON.h>
#include "config.h"
#include "logger.h"

// JSONVar readings;

AsyncWebServer _server = AsyncWebServer(SERVER_PORT);
AsyncWebSocket _ws = AsyncWebSocket("/ws");

//-------------------------------------------------------
static void handleWebSocketMessage(AwsFrameInfo *info, char *data, size_t len)
{
    data[len] = 0;
    String message = (char *)data;
    //  Check if the message is "getReadings"
    // if (strcmp((char*)data, "getReadings") == 0) {
    // if it is, send current sensor readings
    Serial.print("sending to clients: ");
    Serial.println(message);
    _ws.textAll(data, len);
}

static void handleWebSocketMessageBinary(AwsFrameInfo *info, byte *data, size_t len)
{
    for (size_t i = 0; i < info->len; i++)
    {
        Serial.printf("%02x ", data[i]);
    }
    Serial.printf("\n");
}

static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
    else if (type == WS_EVT_PONG)
    {
        // Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }
    else if (type == WS_EVT_DATA)
    {
        // data packet
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len)
        {
            // the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
            if (info->opcode == WS_TEXT)
            {
                // data[len] = 0;
                // Serial.printf("%s\n", (char *)data);
                handleWebSocketMessage(info, (char *)data, len);
                client->text("I got your text message");
            }
            else
            {
                handleWebSocketMessageBinary(info, data, len);
                client->binary("I got your binary message");
            }
        }
        else
        {
            // message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0)
            {
                if (info->num == 0)
                    Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);
            if (info->message_opcode == WS_TEXT)
            {
                data[len] = 0;
                Serial.printf("%s\n", (char *)data);
            }
            else
            {
                for (size_t i = 0; i < len; i++)
                {
                    Serial.printf("%02x ", data[i]);
                }
                Serial.printf("\n");
            }

            if ((info->index + len) == info->len)
            {
                Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if (info->final)
                {
                    Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                    if (info->message_opcode == WS_TEXT)
                        client->text("I got your text message");
                    else
                        client->binary("I got your binary message");
                }
            }
        }
    }
}
//-------------------------------------------------------

class ServerService_Class
{

public:
    ServerService_Class()
    {
    }

    AsyncWebServer &server() const { return _server; }
    AsyncWebSocket &ws() const { return _ws; }

    void begin()
    {
        _ws.onEvent(onEvent);
        _server.addHandler(&_ws);

        // setup endpoints:

        // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        //           { request->send(SPIFFS, "/index.html", "text/html"); });

        // server.serveStatic("/", SPIFFS, "/");

        _server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(200, "text/html", "hello"); });

        //-------------------------------------------------------
        _server.begin();
    }

    void loop()
    {
        _ws.cleanupClients();
    }
};

//-------------------------------------------------------
ServerService_Class ServerService;