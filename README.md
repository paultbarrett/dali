# DALI Libary for Arduino

This Arduino library facilitates communication with a DALI bus, a standard protocol used for controlling lighting systems. DALI (Digital Addressable Lighting Interface) provides a straightforward way to digitally address and control luminaires.

To meet the stringent demands of precise timing, the critical part of the signal processing is offloaded to specialized hardware modules. On the RP2xxx platform, this is achieved using Programmable I/O (PIO), while the ESP platform utilizes the Remote Control (RMT) module. This architecture ensures reliable and accurate data transmission within the DALI system.

Moreover, the library supports the simultaneous operation of multiple buses. However, on the RP2xxx platform, only two buses can be used because only one PIO is available, and its four state machines are fully occupied.

## Using

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



```
Dali::Frame frame;
frame.data = 0xFFFFFFFF;
frame.size = 16; // dali spec allow upto 32 bits
frame.flags = DALI_FRAME_FORWARD; // Only DALI_FRAME_FORWARD will wait for response an call callback
dll1.addTransmitFrame(txFrame);
```

To receive a `Dali::Frame`, there are two types of callbacks: `registerMonitor()` and `registerResponse()`. The `registerMonitor()` callback supports multiple registrations and is invoked whenever a new frame arrives, passing a `Dali::Frame` as its parameter. In contrast, `registerResponse()` only supports a single callback registration, which will be called when a response is expected.

If an error occurs during frame decoding, the callback is also invoked—with both `data` and `size` being empty—and the `flags` field will include the error bit `DALI_FRAME_ERROR`.

If only a single bit is lost, the message may simply be shorter than originally intended. Unfortunately, the DALI standard does not include a CRC or any similar error-checking mechanism.

```
dll.registerMonitor(onDaliFrame);

void onDaliFrame(Dali::Frame frame)
{
    printf("  Monitor: 0x%08X (S: %u - F: %u)\n", frame.data, frame.size, frame.flags);
}
```

The `registerResponse()` callback can only be registered once and is intended for the application layer. It is invoked whenever a transmitted frame is flagged as a forward frame (`DALI_FRAME_FORWARD`), indicating that the data link layer anticipates a potential response. If a corresponding response is received within the appropriate time frame, the frame is treated as a backward frame, and this callback is called with the original forward frame.

```
dll.registerResponse(onDaliResponse);

void onDaliResponse(Dali::Frame txFrame, Dali::Frame rxFrame)
{
    printf("  Response: 0x%08X (S: %u - F: %u) -> TX: 0x%08X (S: %u - F: %u)\n", rxFrame.data, rxFrame.size, rxFrame.flags, txFrame.data, txFrame.size, txFrame.flags);
}
```

### Notice & Todos

Functions such as merging forward frames with backward frames would ideally belong to the network layer. For practical reasons, the current implementation does not separate these two layers, meaning the data link layer is not a pure data link layer. If the need arises in the future, the separation can be revisited.

At present, only the RMT and PIO implementations are available. In theory, a software-based transmitter and an interrupt-based receiver could also be developed. However, since the existing solutions meet the current requirements, these alternatives have not been implemented. Pull requests are, however, very welcome.

Additionally, there is currently no application layer to generate DALI frames and process the responses.

 