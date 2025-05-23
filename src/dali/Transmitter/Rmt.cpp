#ifdef ARDUINO_ARCH_ESP32
#include "Dali/Transmitter/Rmt.h"
#include "Dali/defines.h"
#include "Dali/Rmt_Symbols.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Define ESP-IDF interrupt flags if they're not already defined
#ifndef ESP_INTR_FLAG_LEVEL1
#define ESP_INTR_FLAG_LEVEL1 (1<<1)
#endif
#ifndef ESP_INTR_FLAG_IRAM
#define ESP_INTR_FLAG_IRAM (1<<3)
#endif
#ifndef ESP_INTR_FLAG_LOWMED
#define ESP_INTR_FLAG_LOWMED (1<<9)
#endif

namespace Dali
{
    namespace Transmitter
    {
        Rmt::Rmt(DataLinkLayer *dll, uint pin) : Base(dll, pin)
        {
            _channelConfig = {
                .gpio_num = (gpio_num_t)_pin,
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = 1000000, // 1 MHz resolution (1 microsecond per tick)
                .mem_block_symbols = 64,
                .trans_queue_depth = 4,
                .flags = {
                    .invert_out = false,
                    .with_dma = false, // ESP32 does not support DMA
                }};

            _transmitConfig = (rmt_transmit_config_t){.loop_count = 0};            // Configure RMT channel and ensure its interrupts also run on core 1
            ESP_ERROR_CHECK(rmt_new_tx_channel(&_channelConfig, &_channelHandle));
            
            // Use interrupt flags that match with core 1
            // In Arduino-ESP32 we don't need to handle this explicitly, as we're managing 
            // this through the task pinning and RMT configuration
            
            // Enable RMT channel
            ESP_ERROR_CHECK(rmt_enable(_channelHandle));
        }

        void Rmt::transmitFrame(Frame frame)
        {
            // Manchester-codierte Symbole erstellen
            size_t fullbits = frame.size + 1 + 2;
            rmt_symbol_word_t symbols[fullbits]; // 1 Startbit + 16 Bits * 2 Symbole + 2 Stopbits
            encode(frame, symbols);

            // RMT-Copy-Encoder erstellen
            rmt_copy_encoder_config_t copy_encoder_config = {};
            rmt_encoder_handle_t copy_encoder;
            ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &copy_encoder));

            // Symbole senden
            ESP_ERROR_CHECK(rmt_transmit(_channelHandle, copy_encoder, symbols, sizeof(symbols), &_transmitConfig));

            transmitting(true);
        }

        void Rmt::encode(Frame frame, rmt_symbol_word_t *symbols)
        {
            int bit_count = 0;

            // Start bit (always HIGH for 1 TE)
            symbols[bit_count++] = DALI_SYMBOL_ONE;

            // Encode 16-bit command using Manchester encoding
            for (int i = frame.size - 1; i >= 0; i--)
            {
                // Extract bit value
                int bit = (frame.data >> i) & 0x01;

                // Manchester encoding: Each bit is represented by 2 symbols
                symbols[bit_count++] = bit ? DALI_SYMBOL_ONE : DALI_SYMBOL_ZERO;
            }

            // Stop condition (2 LOW bits for 2 TE)
            symbols[bit_count++] = DALI_SYMBOL_STOP;
            symbols[bit_count++] = DALI_SYMBOL_STOP;
        }
    } // namespace Transmitter

} // namespace Dali
#endif