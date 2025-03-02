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
    // if (!(frame.flags & DALI_FRAME_ECHO))
    // {
    //     Dali::Frame txFrame;
    //     txFrame.flags = DALI_FRAME_BACKWARD;
    //     txFrame.data = frame.data & 0x000000FF;
    //     txFrame.size = 8;
    //     dll1.transmitFrame(txFrame);
    // }
    // digitalWrite(8, HIGH);
    printf("Monitor1: 0x%08X (S: %u - F: %u - T: %u - R: %u)\n", frame.data, frame.size, frame.flags, frame.timestamp, frame.ref);
}
void onDaliFrame2(Dali::Frame frame)
{
    printf("Monitor2: 0x%08X (S: %u - F: %u)\n", frame.data, frame.size, frame.flags);
    // if (frame.data == 0x000010FF)
    // {
    //     // printf("Generate2\n");
    //     Dali::Frame txFrame;
    //     txFrame.flags = DALI_FRAME_BACKWARD;
    //     txFrame.data = frame.data & 0x000000FF;
    //     txFrame.size = 8;
    //     dll2.transmitFrame(txFrame);
    // }
    // else
    // {
    //     // printf("No Generate2\n");
    // }
}

void setup()
{
    // pinMode(8, OUTPUT);
    Serial.begin(115200);
    delay(1000);
    printf("Setup\n");
#ifdef ARDUINO_ARCH_ESP32
    dll1.init(7, 20);
    // dll2.init(14, 13);
#endif
#ifdef ARDUINO_ARCH_RP2040
    dll1.init(17, 16);
    dll2.init(17, 26);
#endif
    dll2.registerMonitor(onDaliFrame2);
    dll1.registerMonitor(onDaliFrame1);
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
    if (millis() - lasttx > 2000)
    {
        lasttx = millis();
        // printf("\n\nSend test frames\n\n");
        Dali::Frame txFrame;
        txFrame.flags = DALI_FRAME_FORWARD;
        txFrame.data = 0x000010FF;
        txFrame.size = 16;
        txFrame.ref = 44;
        dll1.transmitFrame(txFrame);
        for (size_t i = 0; i < 3; i++)
        {
            txFrame.data = 0x000000A0 + i;
            txFrame.size = 8;
            dll1.transmitFrame(txFrame);
        }

        // txFrame.data = (micros() + 1) & 0x0000FFFF;
        // txFrame.size = 16;
        // dll1.transmitFrame(txFrame);
        // txFrame.data = (micros() + 1) & 0x0000FFFF;
        // txFrame.size = 16;
        // dll1.transmitFrame(txFrame);
        // txFrame.data = (micros() + 1) & 0x0000FFFF;
        // txFrame.size = 16;
        // dll1.transmitFrame(txFrame);
        // txFrame.data = (micros() + 1) & 0x0000FFFF;
        // txFrame.size = 16;
        // dll1.transmitFrame(txFrame);
        // txFrame.data = 0x3030;
        // txFrame.size = 16;
        // dll1.transmitFrame(txFrame);
    }
}