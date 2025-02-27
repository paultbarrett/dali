#include "Dali/Receiver/Base.h"
#include "Dali/DataLinkLayer.h"

namespace Dali
{
    namespace Receiver
    {
        bool Base::busy()
        {
            return _busy;
        }

        void Base::process()
        {
            processBusyExpire();
        }

        void Base::processBusyExpire()
        {
            if (!busy()) return;
            if (micros() - _lastTime > DALI_TE_TO_US((2 * 38) + 22)) return;

            busy(false);
        }

        void Base::busy(bool busy)
        {
            _busy = busy;
            _lastTime = micros();
        }

        void Base::receivedFrame(Frame frame)
        {
            _dll->receivedFrame(frame);
        }
    } // namespace Receiver
} // namespace Dali