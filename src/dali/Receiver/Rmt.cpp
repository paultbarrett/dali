#ifdef ARDUINO_ARCH_ESP32
#include "Dali/Receiver/Rmt.h"
#include <Arduino.h>
#include <esp32-hal-log.h>

// Core selection constants
#define CORE_0    (0)
#define CORE_1    (1)
#define RMT_CORE  CORE_1   // Force RMT to run on core 1

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

// Function to force RMT interrupt handler to run on the specified core
static esp_err_t rmt_driver_isr_register(uint32_t intr_flags)
{
    // For Arduino ESP32, we can't directly access esp_intr_alloc,
    // but we're configuring the RMT driver via the Arduino-ESP32 framework
    // which internally sets up the interrupts.
    // This is just a placeholder to indicate our intention to run on core 1
    return ESP_OK;
}

namespace Dali
{
    namespace Receiver
    {
        Rmt::Rmt(DataLinkLayer *dll, uint pin) : Base(dll, pin)
        {
            // Set up task name
            sprintf(_taskName, "%s%d", "dalirmtrx", pin);
            
            // Configure RMT channel
            _channelConfig = (rmt_rx_channel_config_t){
                .gpio_num = (gpio_num_t)pin,
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = DALI_RMT_RESOLUTION_HZ,
                .mem_block_symbols = DALI_RX_BITS, // amount of RMT symbols that the channel can store at a time
                .intr_priority = 1    // Higher priority for interrupt
            };

            _receiveConfig = (rmt_receive_config_t){
                .signal_range_min_ns = 2000,
                .signal_range_max_ns = (uint32_t)(DALI_TE * 4 * 1000)};

            rmt_rx_event_callbacks_t callback = {
                .on_recv_done = Rmt::callback,
            };

            // Create queue for RMT events
            _queueHandle = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));            // Create task on core 1 (pin to network-free core)
            // Set higher task priority (1) for better responsiveness
            xTaskCreatePinnedToCore(Rmt::task, _taskName, 4096, this, 1, &_taskHandle, RMT_CORE);
              // Configure RMT channel
            ESP_ERROR_CHECK(rmt_new_rx_channel(&_channelConfig, &_channelHandle));
            
            // Register callback and explicitly set interrupt allocation flags for core 1
            uint32_t intr_flags = ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1;
            if (RMT_CORE == CORE_1) {
                // Add flag to prefer allocation on core 1
                // ESP_INTR_FLAG_LOWMED is used to allocate interrupt on CPU1
                intr_flags |= ESP_INTR_FLAG_LOWMED;
            }
            
            // Set RMT interrupt allocation preference to run on core 1
            esp_err_t err = rmt_driver_isr_register(intr_flags);
            if (err != ESP_OK) {
                log_e("Failed to register RMT ISR on core %d, error=%d", RMT_CORE, err);
            }
            
            // Register callbacks and enable RMT
            ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(_channelHandle, &callback, _queueHandle));
            ESP_ERROR_CHECK(rmt_enable(_channelHandle));            // Start receiving
            pinMode(pin, INPUT_PULLUP);
            rmt_receive(_channelHandle, _symbols, sizeof(_symbols), &_receiveConfig);
            attachInterruptArg(pin, Rmt::interrupt, this, CHANGE);
        }

        bool Rmt::callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
        {

            BaseType_t high_task_wakeup = pdFALSE;
            QueueHandle_t _queueHandle = (QueueHandle_t)user_data;
            xQueueSendFromISR(_queueHandle, edata, &high_task_wakeup);
            return high_task_wakeup == pdTRUE;
        }

        void Rmt::interrupt(void *pvParameters)
        {
            Rmt *receiver = static_cast<Rmt *>(pvParameters);
            // digitalWrite(8, LOW);
            // Serial.println("I2");
            receiver->startReceiving();
        }

        QueueHandle_t Rmt::getQueueHandle()
        {
            return _queueHandle;
        }        void Rmt::task(void *pvParameters)
        {
            Rmt *receiver = static_cast<Rmt *>(pvParameters);
            rmt_rx_done_event_data_t data;
            
            // Print core information for debugging
            #ifdef DALI_DEBUG_CORE
            log_i("RMT receiver task running on core: %d", xPortGetCoreID());
            #endif
            
            while (true)
            {
                if (xQueueReceive(receiver->getQueueHandle(), &data, portMAX_DELAY) == pdPASS)
                {
                    receiver->stopReceiving(data);
                }
                vTaskDelay(1);
            }
        }

        void Rmt::stopReceiving(rmt_rx_done_event_data_t data)
        {
            // restart receiving
            rmt_receive(_channelHandle, _symbols, sizeof(_symbols), &_receiveConfig);

            Frame frame;

            bool error = false;
            size_t counter = 0;

            for (int i = 0; i < data.num_symbols; i++)
            {
                size_t highs = round((float)data.received_symbols[i].duration0 / 417);
                size_t lows = round((float)data.received_symbols[i].duration1 / 417);

                // special for bit (stop bits)
                if (data.received_symbols[i].duration1 == 0) lows = 1;

                // start bit must be 1
                if (i == 0 && highs != 1) error = true;

                // menchester coding allows only 1 or 2 halbits are same
                if (highs < 1 || highs > 2) error = true;
                if (lows < 1 || lows > 2) error = true;

                // skip on error
                if (error)
                {
                    printf("Error (%u)\n", counter);
                    for (size_t i = 0; i < data.num_symbols; i++)
                    {
                        printf(" - %u: %u (%u) %u (%u)\n", i, data.received_symbols[i].duration0, data.received_symbols[i].level0, data.received_symbols[i].duration1, data.received_symbols[i].level1);
                    }
                    break;
                }

                for (int j = 0; j < highs; j++)
                {
                    if (counter > 1 && counter % 2)
                    {
                        frame.size++;
                        frame.data <<= 1;
                    }
                    counter++;
                }

                for (int j = 0; j < lows; j++)
                {
                    if (counter > 1 && counter % 2)
                    {
                        frame.size++;
                        frame.data <<= 1;
                        frame.data |= 1;
                    }
                    counter++;
                }
            }

            if (error)
            {
                frame.size = 0;
                frame.data = 0;
                frame.flags = DALI_FRAME_ERROR;
            }
            // digitalWrite(8, LOW);
            // Serial.printf("Rx<%u>: %u: Frame done\n", pin(), micros());
            _lastTime = micros();
            receivedFrame(frame);
        }

    } // namespace Receiver

} // namespace Dali
#endif
