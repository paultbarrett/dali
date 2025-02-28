#include "Dali/Receiver/Base.h"
#include "Dali/DataLinkLayer.h"

namespace Dali
{
    namespace Receiver
    {
        bool Base::receiving()
        {
            return _receiving;
        }

        void Base::process()
        {
            expireReceiving();
        }

        void Base::expireReceiving()
        {
            if (!receiving()) return;

            // (1 start + 32 data + 2 stop) * 2 TE + 20 TE pause/reserve
            // in this time the frame should be received
            if (micros() - _lastTime < DALI_TE_TO_US(90)) return;

            _receiving = false;
            // Serial.printf("%u: Expire Receiving \n", micros());
        }

        void Base::receivedFrame(Frame frame)
        {
            _lastTime = micros();
            _receiving = false;
            // Serial.printf("Rx<%u>: %u: Receiving complete: %u \n", _pin, micros(), _receiving);
            _dll->receivedFrame(frame);
        }

        void Base::startReceiving()
        {
            _receiving = true;
            _lastTime = micros();
            // Serial.printf("Rx<%u>: %u: Start Receiving \n", _pin, micros());
        }

        unsigned long Base::lastReceiving()
        {
            return _lastTime;
        }
    } // namespace Receiver
} // namespace Dali