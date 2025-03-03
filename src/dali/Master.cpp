#include "Master.h"

namespace Dali
{
    void Master::init(uint tx, uint rx)
    {
        _dll.init(tx, rx);
    }

    void Master::process()
    {
        _dll.process();
    }

    void Master::registerMonitor(std::function<void(Frame)> callback)
    {
        _dll.registerMonitor(callback);
    }

    void Master::sendArc(uint8_t address, uint8_t value, bool isGroup)
    {
        Frame frame;
        frame.data = prepareCommand16(isGroup, address, false, value);
        frame.flags = DALI_FRAME_FORWARD;
        frame.size = 16;
        _dll.transmitFrame(frame);
    }

    uint32_t Master::sendCommand(uint8_t address, uint8_t command, bool isGroup, bool response)
    {
        Frame frame;
        frame.data = prepareCommand16(isGroup, address, true, command);
        frame.flags = DALI_FRAME_FORWARD;
        frame.size = 16;
        frame.ref = micros();
        _dll.transmitFrame(frame);
        return frame.ref;
    }

    uint32_t Master::sendSpecialCommand(uint8_t command, uint8_t value, bool response)
    {
        command = command & 0x0F;
        command |= 1 << 5;
        Frame frame;
        frame.data = prepareCommand16(false, command, true, value);
        frame.flags = DALI_FRAME_FORWARD;
        frame.size = 16;
        frame.ref = micros();
        _dll.transmitFrame(frame);
        return frame.ref;
    }

    uint32_t Master::prepareCommand16(bool isGroup, uint8_t byte0, bool selector, uint8_t byte1)
    {
        uint32_t data = 0;
        data |= (isGroup ? 1 : 0) << 15;
        data |= (byte0 & 0x3F) << 9;
        data |= (selector ? 1 : 0) << 8;
        data |= (byte1 & 0xFF);
        return data;
    }
};