#ifdef ARDUINO_ARCH_ESP32
#include "Dali/Receiver/Rmt.h"
namespace Dali
{
    namespace Receiver
    {
        Rmt::Rmt(DataLinkLayer *dll, uint pin) : Base(dll, pin)
        {
            // digitalWrite(8, HIGH);
            sprintf(_taskName, "%s%d", "dalirmtrx", pin);
            _channelConfig = (rmt_rx_channel_config_t){
                .gpio_num = (gpio_num_t)pin,
                .clk_src = RMT_CLK_SRC_DEFAULT,
                .resolution_hz = DALI_RMT_RESOLUTION_HZ,
                .mem_block_symbols = DALI_RX_BITS, // amount of RMT symbols that the channel can store at a time
                .flags = {
                    // The inversion is conceptually incorrect and not inverted. However, the used Manchester encoding is inverted and simplifies the evaluation.
                    .invert_in = true}};

            _receiveConfig = (rmt_receive_config_t){
                .signal_range_min_ns = 1000,
                .signal_range_max_ns = 900000};

            rmt_rx_event_callbacks_t callback = {
                .on_recv_done = Rmt::callback,
            };

            _queueHandle = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));

            xTaskCreate(Rmt::task, _taskName, 4096, this, 0, &_taskHandle);
            ESP_ERROR_CHECK(rmt_new_rx_channel(&_channelConfig, &_channelHandle));
            ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(_channelHandle, &callback, _queueHandle));
            ESP_ERROR_CHECK(rmt_enable(_channelHandle));
            rmt_receive(_channelHandle, _symbols, sizeof(_symbols), &_receiveConfig);

            gpio_config_t io_conf = {};
            io_conf.intr_type = _channelConfig.flags.invert_in ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = BIT64(_channelConfig.gpio_num);
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            // Configure the pin
            gpio_config(&io_conf);
            // printf("gpio_config:                     %d (%s)\n", resp, esp_err_to_name(resp));
            //  Configure the interrupt
            gpio_install_isr_service(0 /* No flags */); // Call this only once !!
            // printf("gpio_install_isr_service:        %d (%s)\n", resp, esp_err_to_name(resp));
            gpio_isr_handler_add(_channelConfig.gpio_num, Rmt::interrupt, this);
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
            receiver->startReceiving();
        }

        QueueHandle_t Rmt::getQueueHandle()
        {
            return _queueHandle;
        }

        void Rmt::task(void *pvParameters)
        {
            Rmt *receiver = static_cast<Rmt *>(pvParameters);
            rmt_rx_done_event_data_t data;

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

            // printf("Found\n");
            // for (size_t i = 0; i < data.num_symbols; i++)
            // {
            //     printf(" - %u: %u (%u) %u (%u)\n", i, data.received_symbols[i].duration0, data.received_symbols[i].level0, data.received_symbols[i].duration1, data.received_symbols[i].level1);
            // }

            Frame frame;

            bool error = false;
            uint8_t counter = 0;

            for (int i = 0; i < data.num_symbols; i++)
            {
                uint8_t count0 = 0;
                uint8_t count1 = 0;
                const uint16_t duration0 = data.received_symbols[i].duration0;
                const uint16_t duration1 = data.received_symbols[i].duration1;
                const bool level0 = data.received_symbols[i].level0;
                const bool level1 = data.received_symbols[i].level1;

                // check timing duration0
                if (duration0 >= DALI_THRESHOLD_1TE_LOW && duration0 <= DALI_THRESHOLD_1TE_HIGH)
                    count0 = 1;
                else if (duration0 >= DALI_THRESHOLD_2TE_LOW && duration0 <= DALI_THRESHOLD_2TE_HIGH)
                    count0 = 2;
                else
                    error = true;

                // check timing duration1
                if (duration1 == 0)
                    count1 = 1;
                else if (duration1 >= DALI_THRESHOLD_1TE_LOW && duration1 <= DALI_THRESHOLD_1TE_HIGH)
                    count1 = 1;
                else if (duration1 >= DALI_THRESHOLD_2TE_LOW && duration1 <= DALI_THRESHOLD_2TE_HIGH)
                    count1 = 2;
                else
                    error = true;

                if (i == 0 && count0 != 1) // start bit must be 1
                    error = true;

                if (error) break;

                for (int j = 0; j < count0; j++)
                {
                    if (counter > 1 && counter % 2)
                    {
                        frame.size++;
                        frame.data <<= 1;
                        frame.data |= level0;
                    }
                    counter++;
                }

                for (int j = 0; j < count1; j++)
                {
                    if (counter > 1 && counter % 2)
                    {
                        frame.size++;
                        frame.data <<= 1;
                        frame.data |= level1;
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