#include "Arduino.h"
#include "Dali/DataLinkLayer.h"

Dali::DataLinkLayer dll1;
Dali::DataLinkLayer dll2;

uint32_t inttime = 0;
uint8_t tdata;
bool tstatus = false;
uint32_t lasttx = 0;

#ifdef ARDUINO_ARCH_RP2040
#define printf Serial.printf
#endif

void onDaliFrame1(Dali::Frame frame)
{
    printf("Monitor1: 0x%08X (S: %u - F: %u)\n", frame.data, frame.size, frame.flags);
}
void onDaliResponse1(Dali::Frame txFrame, Dali::Frame rxFrame)
{
    printf("Response1: 0x%08X (S: %u - F: %u) -> TX: 0x%08X (S: %u - F: %u)\n", rxFrame.data, rxFrame.size, rxFrame.flags, txFrame.data, txFrame.size, txFrame.flags);
}
void onDaliFrame2(Dali::Frame frame)
{
    printf("Monitor2: 0x%08X (S: %u - F: %u)\n", frame.data, frame.size, frame.flags);
    // if (!(frame.flags & 0b00000100))
    // {
    //     printf("Generate2\n");
    //     Dali::Frame txFrame;
    //     txFrame.flags = DALI_FRAME_BACKWARD;
    //     txFrame.data = frame.data & 0x000000FF;
    //     txFrame.size = 8;
    //     dll2.transmitFrame(txFrame);
    // }
    // else
    // {
    //     printf("No Generate2\n");
    // }
}
void onDaliResponse2(Dali::Frame txFrame, Dali::Frame rxFrame)
{
    printf("Response2: 0x%08X (S: %u - F: %u) -> TX: 0x%08X (S: %u - F: %u)\n", rxFrame.data, rxFrame.size, rxFrame.flags, txFrame.data, txFrame.size, txFrame.flags);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    printf("Setup\n");
#ifdef ARDUINO_ARCH_ESP32
    dll1.init(7, 20);
    dll2.init(14, 13);
#endif
#ifdef ARDUINO_ARCH_RP2040
    dll1.init(17, 16);
    dll2.init(17, 26);
#endif
    dll2.registerMonitor(onDaliFrame2);
    dll2.registerResponse(onDaliResponse2);
    dll1.registerMonitor(onDaliFrame1);
    dll1.registerResponse(onDaliResponse1);
    printf("Setup done\n");
}
uint32_t t = 0;
void loop()
{
    dll1.process();
    dll2.process();

    // if (millis() - t > 5000)
    // {
    //     printf("loop\n");
    //     t = millis();
    // }
    if (millis() - lasttx > 3000)
    {
        printf("\n\nSend test frames\n\n");
        lasttx = millis();
        Dali::Frame txFrame;
        txFrame.flags = DALI_FRAME_FORWARD;
        txFrame.data = micros() & 0x0000FFFF;
        txFrame.size = 16;
        dll1.transmitFrame(txFrame);
        txFrame.data = (micros() + 1) & 0x0000FFFF;
        txFrame.size = 16;
        dll1.transmitFrame(txFrame);
        txFrame.data = (micros() + 1) & 0x0000FFFF;
        txFrame.size = 16;
        dll1.transmitFrame(txFrame);
        txFrame.data = (micros() + 1) & 0x0000FFFF;
        txFrame.size = 16;
        dll1.transmitFrame(txFrame);
        txFrame.data = (micros() + 1) & 0x0000FFFF;
        txFrame.size = 16;
        dll1.transmitFrame(txFrame);
        txFrame.data = (micros() + 1) & 0x0000FFFF;
        txFrame.size = 16;
        dll1.transmitFrame(txFrame);
        // txFrame.data = 0x3030;
        // txFrame.size = 16;
        // dll1.transmitFrame(txFrame);
    }
}