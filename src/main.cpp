#include "Arduino.h"
#include "Dali/Master.h"

Dali::Master m1;
Dali::Master m2;

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
    m1.init(7, 20);
    m2.init(5, 8);
#endif
#ifdef ARDUINO_ARCH_RP2040
    m1.init(17, 16);
    m2.init(26, 18);
#endif
    m1.registerMonitor(onDaliFrame1);
    m2.registerMonitor(onDaliFrame2);
    printf("Setup done\n");
}

uint32_t t = 0;
uint8_t arc = 0;
void loop()
{
    m1.process();
    m2.process();

    if (millis() - lasttx > 2000)
    {
        lasttx = millis();
        if(arc == 0)
            arc = 0xFE;
        else
            arc = 0;
        // printf("\n\nSend test frames\n\n");
        
        for (size_t i = 0; i < 3; i++)
        {
            m1.sendArc(i, arc);
        }

        for (size_t i = 0; i < 3; i++)
        {
            m2.sendArc(i, arc);
        }
    }
}