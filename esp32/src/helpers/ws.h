#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

typedef void (*websocketMessageTextHandler_t)(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsFrameInfo *info, char *data, size_t len);
typedef void (*websocketMessageBinHandler_t)(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsFrameInfo *info, uint8_t *data, size_t len);
typedef void (*websocketConnectHandler_t)(AsyncWebSocket *server, AsyncWebSocketClient *client);
typedef void (*websocketDisconnectHandler_t)(AsyncWebSocket *server, AsyncWebSocketClient *client);
typedef void (*websocketErrorHandler_t)(AsyncWebSocket *server, AsyncWebSocketClient *client, uint16_t code, char *data);
typedef void (*websocketPongHandler_t)(AsyncWebSocket *server, AsyncWebSocketClient *client);

//-------------------------------------------------------

class WsHandler
{
private:
    AsyncWebSocket *_ws;

    AsyncWebServer *_server = NULL;

    websocketMessageTextHandler_t _messageTextHandler = NULL;
    websocketMessageBinHandler_t _messageBinHandler = NULL;
    websocketConnectHandler_t _connectHandler = NULL;
    websocketDisconnectHandler_t _disconnectHandler = NULL;
    websocketErrorHandler_t _errorHandler = NULL;
    websocketPongHandler_t _pongHandler = NULL;

public:
    WsHandler()
    {
    }
    ~WsHandler() {}

    AsyncWebSocket *ws() { return _ws; }

    void begin(AsyncWebServer *server, const String &url)
    {
        this->_server = server;
        this->_ws = new AsyncWebSocket(url);

        this->_ws->onEvent(
            [this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void
            {
                this->_onEvent(server, client, type, arg, data, len);
            });
        this->_server->addHandler(this->_ws);
    }

    void end()
    {
        this->_server->removeHandler(this->_ws);
        delete (this->_ws);
    }

    void cleanupClients()
    {
        if (this->_ws != NULL)
        {
            this->_ws->cleanupClients();
        }
    }

    void messageTextHandler(websocketMessageTextHandler_t messageTextHandler) { this->_messageTextHandler = messageTextHandler; }
    void messageBinHandler(websocketMessageBinHandler_t messageBinHandler) { this->_messageBinHandler = messageBinHandler; }
    void connectHandler(websocketConnectHandler_t connectHandler) { this->_connectHandler = connectHandler; }
    void disconnectHandler(websocketDisconnectHandler_t disconnectHandler) { this->_disconnectHandler = disconnectHandler; }
    void errorHandler(websocketErrorHandler_t errorHandler) { this->_errorHandler = errorHandler; }
    void pongHandler(websocketPongHandler_t pongHandler) { this->_pongHandler = pongHandler; }

private:
    void _onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {

        if (type == WS_EVT_CONNECT)
        {
            if (_connectHandler != NULL)
                _connectHandler(server, client);
            client->ping();
        }
        else if (type == WS_EVT_DISCONNECT)
        {
            if (_disconnectHandler != NULL)
                _disconnectHandler(server, client);
        }
        else if (type == WS_EVT_ERROR)
        {
            if (_errorHandler != NULL)
                _errorHandler(server, client, *((uint16_t *)arg), (char *)data);
        }
        else if (type == WS_EVT_PONG)
        {
            if (_pongHandler != NULL)
                _pongHandler(server, client);
        }
        else if (type == WS_EVT_DATA)
        {
            // data packet
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            if (info->final && info->index == 0 && info->len == len)
            {
                // the whole message is in a single frame and we got all of it's data
                if (info->opcode == WS_TEXT)
                {
                    // data[len] = 0;
                    // Serial.printf("%s\n", (char *)data);
                    if (_messageTextHandler != NULL)
                        _messageTextHandler(server, client, info, (char *)data, len);
                }
                else
                {
                    if (_messageBinHandler != NULL)
                        _messageBinHandler(server, client, info, data, len);
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
};