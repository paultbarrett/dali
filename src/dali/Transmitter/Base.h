#pragma once
#include "Dali/Frame.h"
#include "Dali/defines.h"
#include <Arduino.h>

namespace Dali
{
    namespace Transmitter
    {
        class Base
        {
          protected:
            uint _pin;

          public:
            Base(uint pin) : _pin(pin) {}
            virtual void transmit(Frame frame) = 0;
            virtual void process() {};
        };
    } // namespace Transmitter

} // namespace Dali