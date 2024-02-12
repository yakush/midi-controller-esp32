#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "SPIFFS.h"
#include "config.h"
#include "logger.h"
#include "state/midiState.h"
#include "state/appState.h"
#include "waveGenerators.h"
#include "synthesizerService.h"
#include "helpers/ws.h"
#include "helpers/AsyncSinglePageAppHandler.h"

AsyncWebServer _server = AsyncWebServer(SERVER_PORT);
WsHandler _wsHandler;

//-------------------------------------------------------
static void handleWebSocketMessage(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsFrameInfo *info, char *data, size_t len)
{
    data[len] = 0;
    String message = (char *)data;
    //  Check if the message is "getReadings"
    // if (strcmp((char*)data, "getReadings") == 0) {
    // if it is, send current sensor readings
    Serial.print("sending to clients: ");
    Serial.println(message);
    server->textAll(data, len);
}

static void handleWebSocketMessageBinary(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsFrameInfo *info, uint8_t *data, size_t len)
{
    for (size_t i = 0; i < info->len; i++)
    {
        Serial.printf("%02x ", data[i]);
    }
    Serial.printf("\n");
}

static void handleWebSocket_connect(AsyncWebSocket *server, AsyncWebSocketClient *client)
{
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
}
static void handleWebSocket_disconnect(AsyncWebSocket *server, AsyncWebSocketClient *client)
{
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
}
static void handleWebSocket_error(AsyncWebSocket *server, AsyncWebSocketClient *client, uint16_t code, char *data)
{
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), code, data);
}
static void handleWebSocket_pong(AsyncWebSocket *server, AsyncWebSocketClient *client)
{
    // Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
}

//-------------------------------------------------------

class ServerService_Class
{

public:
    ServerService_Class()
    {
    }

    AsyncWebServer *server() const { return &_server; }
    AsyncWebSocket *ws() const { return _wsHandler.ws(); }

    void begin()
    {
        SPIFFS.begin();

        _wsHandler.begin(&_server, "/ws");
        _wsHandler.messageTextHandler(handleWebSocketMessage);
        _wsHandler.messageBinHandler(handleWebSocketMessageBinary);

        _wsHandler.connectHandler(handleWebSocket_connect);
        _wsHandler.disconnectHandler(handleWebSocket_disconnect);
        _wsHandler.errorHandler(handleWebSocket_error);
        _wsHandler.pongHandler(handleWebSocket_pong);

        // setup endpoints:

        // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        //           { request->send(SPIFFS, "/index.html", "text/html"); });

        // server.serveStatic("/", SPIFFS, "/");

        //-------------------------------------------------------
        // ALL NotFound
        _server.onNotFound([](AsyncWebServerRequest *request)
                           { request->send(404, "text/html", "no such page"); });

        //-------------------------------------------------------
        // static
        {
            auto handler = new AsyncSinglePageAppHandler("/web/", SPIFFS, "/web/", "");
            handler->setDefaultFile("index.html");
            _server.addHandler(handler);

            //_server.serveStatic("/web/", SPIFFS, "/web/").setDefaultFile("index.html");
        }
        //-------------------------------------------------------
        // GET /test
        _server.on(
            "/test", HTTP_GET,
            [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", "hello"); });

        //-------------------------------------------------------
        // GET + POST /midi-state
        _server.on(
            "/midi-state", HTTP_GET,
            [](AsyncWebServerRequest *request)
            {
                AsyncJsonResponse *response = new AsyncJsonResponse();
                auto root = response->getRoot();
                MidiState.writeToJson(root);
                response->setLength();
                request->send(response);
            });

        AsyncCallbackJsonWebHandler *handler_midi_state = new AsyncCallbackJsonWebHandler(
            "/midi-state", [](AsyncWebServerRequest *request, JsonVariant &json)
            {
                MidiState.readFromJson(json,true);

                //reply with new state
                AsyncJsonResponse *response = new AsyncJsonResponse();
                auto root = response->getRoot();
                MidiState.writeToJson(root);
                response->setLength();
                request->send(response);
            ; });
        _server.addHandler(handler_midi_state);

        {
            auto handler = new AsyncCallbackJsonWebHandler(
                "/wave", [](AsyncWebServerRequest *request, JsonVariant &json)
                {
                bool okToSet=false;
                WaveType type;
                size_t resolution=100;

                auto jsonObj = json.as<JsonObject>();

                if (jsonObj.containsKey("resolution"))
                {
                    resolution = jsonObj["resolution"];
                } 
                if (jsonObj.containsKey("type"))
                {
                    String typeStr = jsonObj["type"];                        
                    if (typeStr == "sin"){
                        type =WaveType::SIN;
                        okToSet=true;
                    }else if (typeStr == "sawtooth"){
                        type =WaveType::SAWTOOTH;
                        okToSet=true;
                    }
                }
                
                if (!okToSet){
                    request->send(200, "text/html", "error in params"); 
                    return;
                }
                SynthesizerService.setWave(type,resolution);
                request->send(200, "text/html", "ok"); 

            ; });
            _server.addHandler(handler);
        }
        //-------------------------------------------------------
        //-------------------------------------------------------

        _server.begin();
    }

    void loop()
    {
        _wsHandler.cleanupClients();
    }
};

//-------------------------------------------------------
ServerService_Class ServerService;