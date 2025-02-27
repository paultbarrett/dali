#include "Dali/DataLinkLayer.h"

#ifdef ARDUINO_ARCH_ESP32
#include "Dali/Receiver/Rmt.h"
#include "Dali/Transmitter/Rmt.h"
#endif
#ifdef ARDUINO_ARCH_RP2040
#include "Dali/Receiver/Pio.h"
#include "Dali/Transmitter/Pio.h"
#endif

namespace Dali
{
    void DataLinkLayer::addTransmitFrame(Frame frame)
    {
        _txQueue.push(frame);
    };

    void DataLinkLayer::receivedFrame(Frame frame)
    {
        _rxLastTime = micros();

        // sollte die antwort nicht innerhalb von 22TE kommen, dann ist es auch keine antwort
        if (micros() - _txLastTime < DALI_TE_TO_US(22))
        {
            _nextResponse = false;
        }

        if (_nextResponse)
        {
            frame.flags |= DALI_FRAME_BACKWARD;
            _nextResponse = false;
        }

        if (_isTransmitting && _txFrame.data == frame.data && _txFrame.size == frame.size)
        {
            _isTransmitting = false;
            if ((_txFrame.flags & DALI_FRAME_FORWARD) == DALI_FRAME_FORWARD)
            {
                frame.flags |= DALI_FRAME_FORWARD;
                _nextResponse = true;
            }
        }
        _rxQueue.push(frame);
    };

    void DataLinkLayer::init(uint tx, uint rx)
    {
#ifdef ARDUINO_ARCH_ESP32
        _rxReceiver = new Dali::Receiver::Rmt(this, rx);
        _txTransmitter = new Dali::Transmitter::Rmt(tx);
#endif
#ifdef ARDUINO_ARCH_RP2040
        _rxReceiver = new Dali::Receiver::Pio(this, rx);
        _txTransmitter = new Dali::Transmitter::Pio(tx);
#endif
    };

    void DataLinkLayer::process()
    {
        _rxReceiver->process();
        _txTransmitter->process();
        processReceive();
        processTransmit();

#ifdef DALI_SIMULATE_RESPONSE
        if (_hack && (micros() - _rxLastTime) >= DALI_TE_TO_US(7))
        {
            // printf(" > simulate answer\n");
            Frame frame;
            frame.data = 0xFE;
            frame.size = 8;
            _txTransmitter->transmit(frame);
            _hack = false;
            _rxLastTime = micros();
        }
#endif
    };

    void DataLinkLayer::processReceive()
    {
        if (_rxQueue.empty()) return;

        Frame rxFrame = _rxQueue.front();
        _rxQueue.pop();

        // Serial.printf("  RxFrame: 0x%08X (S: %u - F: %u)\n", rxFrame.data, rxFrame.size, rxFrame.flags);

        for (std::function<void(Frame)> &callback : _callbackMonitors)
            callback(rxFrame);

        if (rxFrame.flags & DALI_FRAME_BACKWARD && rxFrame.flags != DALI_FRAME_ERROR)
            _callbackResponse(_txFrame, rxFrame);

        if (rxFrame.flags & DALI_FRAME_FORWARD) _hack = true;
    }

    void DataLinkLayer::processTransmit()
    {
        if (_rxReceiver->busy()) return;
        if (_txQueue.empty()) return;

        const unsigned long current = micros();
        const unsigned long rxLastTime = _rxLastTime;
        const unsigned long txLastTime = _txLastTime;

        if ((current - rxLastTime) < DALI_TE_TO_US(22)) return;
        if ((current - txLastTime) < DALI_TE_TO_US(22)) return;

        _txFrame = _txQueue.front();
        _txQueue.pop();

        // Serial.printf("  TxFrame: 0x%08X (pause %uus %uus)\n", _txFrame.data, (unsigned long)(current - rxLastTime), (unsigned long)(current - txLastTime));
        _txTransmitter->transmit(_txFrame);
        _isTransmitting = true;
        _nextResponse = false;
        _txLastTime = micros();
        _rxLastTime = micros();
    }

    void DataLinkLayer::registerMonitor(std::function<void(Frame)> callback)
    {
        _callbackMonitors.push_back(callback);
    }

    void DataLinkLayer::registerResponse(std::function<void(Frame, Frame)> callback)
    {
        _callbackResponse = callback;
    }
}; // namespace Dali
