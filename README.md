# DALI Libary for Arduino

This Arduino library facilitates communication with a DALI bus, a standard protocol used for controlling lighting systems. DALI (Digital Addressable Lighting Interface) provides a straightforward way to digitally address and control luminaires.

To meet the stringent demands of precise timing, the critical part of the signal processing is offloaded to specialized hardware modules. On the RP2xxx platform, this is achieved using Programmable I/O (PIO), while the ESP platform utilizes the Remote Control (RMT) module. This architecture ensures reliable and accurate data transmission within the DALI system.

Moreover, the library supports the simultaneous operation of multiple buses. However, on the RP2xxx platform, only two buses can be used because only one PIO is available, and its four state machines are fully occupied.

## DataLinkLayer

Inialize a datalinklayer

```
Dali::DataLinkLayer dll;

void setup() {
    dll.init(TX_PIN, RX_PIN);
}

void loop() {
    dll.process();
}
```

To send a frame, a `Dali::Frame` must be created and populated. The raw data is stored in the `data` field, and the `size` field specifies its length in bits (up to 32 bits). Since the frame type cannot be determined directly, additional information can be stored in the `flags` field.

For example, if you are implementing the gateway yourself, the `DALI_FRAME_FORWARD` flag should be set; only then will the response callback be invoked.

Additionally, a reference can be provided that will be carried over to the received frame, including any potential backward frame.

```
Dali::Frame frame;
frame.data = 0xFFFFFFFF;
frame.size = 16; // dali spec allow upto 32 bits
frame.flags = DALI_FRAME_FORWARD; // Only DALI_FRAME_FORWARD will wait for response an call callback
frame.ref = 1234 // A reference that is adopted upon receiving the telegram. It is also carried over to any potential backward frame.
dll1.addTransmitFrame(txFrame);
```

To receive a `Dali::Frame`, multiple callbacks can be registered using `registerMonitor()`. All registered callbacks are invoked when a frame arrives, and each receives the corresponding `Dali::Frame`. The frame’s actual data is stored in the `data` and `size` fields.

Additionally, a timestamp (obtained via `micros()`) is recorded in the `Dali::Frame`. If the frame was sent by the same device, the reference to the sent frame is also carried over, which applies similarly to any potential backward frame.

The `flags` field provides further information about the frame. For example, `DALI_FRAME_ECHO` indicates that the frame was self-transmitted, while `DALI_FRAME_ERROR` is set when the received frame is faulty. Another flag, `DALI_FRAME_COLLISION`, signals that a transmitted frame was not immediately received, indicating a collision.

Keep in mind that frames cannot always be reliably flagged as faulty because there is no CRC or equivalent mechanism to definitively verify their integrity. In such cases, a frame may simply be shorter than expected. Therefore, it is advisable to verify the frame length at the application layer.

```
dll.registerMonitor(onDaliFrame);

void onDaliFrame(Dali::Frame frame)
{
    printf("  Monitor: 0x%08X (S: %u - F: %u)\n", frame.data, frame.size, frame.flags);
}
```

### Notice & Todos

At present, only the RMT and PIO implementations are available. In theory, a software-based transmitter and an interrupt-based receiver could also be developed. However, since the existing solutions meet the current requirements, these alternatives have not been implemented. Pull requests are, however, very welcome.

 