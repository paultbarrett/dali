#pragma once
#include "Dali/Frame.h"
#include "Dali/defines.h"
#include <Arduino.h>

namespace Dali
{
    class DataLinkLayer;

    namespace Receiver
    {
        class Base
        {
            DataLinkLayer *_dll;

          protected:
            uint _pin;
            bool _busy = false;
            unsigned long _lastTime = 0;

            void receivedFrame(Frame frame);

          public:
            Base(DataLinkLayer *dll, uint pin) : _dll(dll), _pin(pin) {}
            bool busy();
            void busy(bool busy);
            void processBusyExpire();
            virtual void process();
        };
    } // namespace Receiver

} // namespace Dali