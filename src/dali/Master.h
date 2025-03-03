#pragma once
#include "Arduino.h"
#include "Commands.h"
#include "Dali/Frame.h"
#include "DataLinkLayer.h"

namespace Dali
{
    class Master
    {
        DataLinkLayer _dll;

        public:
            void init(uint tx, uint rx);
            void process();

            void registerMonitor(std::function<void(Frame)> callback);

            /** Send an arc command to an evg/group (0-254). */
            void sendArc(uint8_t address, uint8_t value, bool isGroup = false);
            /** Send a command to an evg/group. Returns ref. */
            uint32_t sendCommand(uint8_t address, uint8_t command, bool isGroup = false, bool response = false);
            /** Send a special command to an evg/group. Returns ref. */
            uint32_t sendSpecialCommand(uint8_t command, uint8_t value = 0x00, bool response = false);

            /** Prepare a command with 16 bits. */
            uint32_t prepareCommand16(bool isGroup, uint8_t address, bool selector, uint8_t command);
    };
}; // namespace Dali