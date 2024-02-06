#pragma once

#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "state/midiState.h"
#include "state/appState.h"
#include "types/midi.h"
#include "types/i2sFrame.h"
#include "logger.h"
#include "synthesizerService.h"

//-------------------------------------------------------
#define SAMPLE_RATE (44100)
// #define DMA_BUF_LEN (32)
// #define DMA_NUM_BUF (2)
#define DMA_BUF_LEN (64)
#define DMA_NUM_BUF (8)
#define I2S_NUM (0)
#define WAVE_FREQ_HZ (235.0f)
#define TWOPI (6.28318531f)
#define PHASE_INC (TWOPI * WAVE_FREQ_HZ / SAMPLE_RATE)

// forward
static void audio_task(void *userData);

//-------------------------------------------------------
class SpeakerService_CLASS
{

private:
    int _freq;
    i2s_port_t _port;
    i2s_config_t _i2s_config;
    i2s_pin_config_t _pin_config;

    I2S_Frame out_buf[DMA_BUF_LEN];

public:
    SpeakerService_CLASS() {}
    virtual ~SpeakerService_CLASS() {}
    bool begin(int port,
               int pin_bck,
               int pin_lrck,
               int pin_data,
               int freq)
    {
        this->_freq = freq;
        this->_port = (i2s_port_t)port;

        _i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = freq,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            //.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
            .dma_buf_count = DMA_NUM_BUF,
            .dma_buf_len = DMA_BUF_LEN // Interrupt level 1
        };

        _pin_config = {
            .bck_io_num = pin_bck,    // this is BCK pin
            .ws_io_num = pin_lrck,    // this is LRCK pin
            .data_out_num = pin_data, // this is DATA output pin
            .data_in_num = -1         // Not used
        };

        bool ok = true;
        ok = ok && i2s_driver_install((i2s_port_t)_port, &_i2s_config, 0, NULL) == ESP_OK;
        ok = ok && i2s_set_pin(_port, &_pin_config) == ESP_OK;
        ok = ok && i2s_set_sample_rates(_port, freq) == ESP_OK;
        // return ok;

        // Highest possible priority for realtime audio task
        xTaskCreate(audio_task, "audio", 1024, NULL, configMAX_PRIORITIES - 1, NULL);

        return ok;
    }

    void loop()
    {
    }

    void run()
    {
        // TODO: remember last bytes_written and adjust how many to request?
        
        uint64_t start = esp_timer_get_time();
        SynthesizerService.writeBuffer(this->out_buf, DMA_BUF_LEN);
        uint64_t end = esp_timer_get_time();
        AppState.i2s_writeTime(end-start);

        size_t bytes_written;
        i2s_write(_port, out_buf, sizeof(out_buf), &bytes_written, (TickType_t)portMAX_DELAY);

        // delay(1);
        taskYIELD();
    }
};

//-----------------------------------------------------------------
// global :
SpeakerService_CLASS SpeakerService;
// extern ns_speakerService::speakerService_CLASS speakerService;
//-----------------------------------------------------------------

static void audio_task(void *userData)
{
    for (;;)
    {
        SpeakerService.run();
    }
}