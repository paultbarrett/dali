#pragma once
#include <Arduino.h>
#include <functional>
#include <queue>

#include "Dali/Frame.h"
#include "Dali/Receiver/Base.h"
#include "Dali/Transmitter/Base.h"

namespace Dali
{
    class DataLinkLayer
    {
        friend class Receiver::Base;

        std::vector<std::function<void(Frame)>> _callbackMonitors;
        std::function<void(Frame, Frame)> _callbackResponse;

        std::queue<Frame> _txQueue;
        std::queue<Frame> _rxQueue;

        Frame _txFrame;

        Transmitter::Base *_txTransmitter = nullptr;
        Receiver::Base *_rxReceiver = nullptr;

        unsigned long _rxLastTime = 0;
        unsigned long _txLastTime = 0;
        bool _nextResponse = false;
        bool _isTransmitting = false;
        bool _hack = false;

        void processReceive();
        void processTransmit();

      public:
        void addTransmitFrame(Frame frame);
        void receivedFrame(Frame frame);
        void init(uint tx, uint rx);
        void process();
        void registerMonitor(std::function<void(Frame)> callback);
        void registerResponse(std::function<void(Frame, Frame)> callback);
    };

}; // namespace Dali
