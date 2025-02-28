#ifdef ARDUINO_ARCH_RP2040
#include "Dali/Receiver/Pio.h"
#include "Dali/Pio/StateMachines.h"
namespace Dali
{
    namespace Receiver
    {
        Pio::Pio(DataLinkLayer *dll, uint pin) : Base(dll, pin)
        {
            _sm = DaliPioManager.rxStateMachine();
            dali_rx_program_init(DaliPioManager.pio(), _sm, DaliPioManager.rxProgramOffset(), pin);
            DaliPioManager.setInterruptHandler(_sm, std::bind(&Pio::interrupt, this));
        }

        void Pio::process()
        {
            while (!pio_sm_is_rx_fifo_empty(DaliPioManager.pio(), _sm))
            {
                uint32_t data = pio_sm_get(DaliPioManager.pio(), _sm);
                if (bufferIndex == 0 && !data) continue; // skip leading zeros from previous frame
                // Serial.printf("Rx<%u>: %u: Data: %u\n", _pin, micros(), data);
                buffer[bufferIndex++] = data & 0b11;
                _zeros = (data == 0 ? _zeros - 1 : 2);
            }

            if (!_zeros)
            {
                _zeros = 2;

                Frame frame;

                for (size_t i = 0; i < bufferIndex; i++)
                {
                    if (buffer[i] == 0b11)
                    {
                        frame.flags = DALI_FRAME_ERROR;
                        break;
                    }
                    if (buffer[i] == 0b00)
                    {
                        if (i < bufferIndex - 1 && buffer[i + 1] == 0b00)
                        {
                            // 2 STOPBITS
                            break;
                        }

                        frame.flags = DALI_FRAME_ERROR;
                        break;
                    }

                    if (i == 0)
                    {
                        if (buffer[i] != 0b10)
                        {
                            frame.flags = DALI_FRAME_ERROR;
                            break;
                        }

                        // STARTBIT
                        continue;
                    }

                    frame.data <<= 1;
                    frame.data |= buffer[i] == 0b10 ? 1 : 0;
                    frame.size++;
                }

                // reset partial data on error
                if (frame.flags == DALI_FRAME_ERROR)
                {
                    frame.size = 0;
                    frame.data = 0;
                }

                bufferIndex = 0;
                receivedFrame(frame);
            }

            Base::process();
        }

        void Pio::interrupt()
        {
            startReceiving();
        }
    } // namespace Pio
} // namespace Dali
#endif