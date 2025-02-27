#include "Dali/Frame.h"
#include "Dali/Pio/Manager.h"
#include "Dali/Transmitter/Base.h"

namespace Dali
{
    namespace Transmitter
    {
        class Pio : public Base
        {
            uint _sm;

          public:
            Pio(uint pin);
            void transmit(Frame frame) override;
        };
    } // namespace Transmitter
} // namespace Dali