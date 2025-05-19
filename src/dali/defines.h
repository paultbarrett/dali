#define DALI_TE (1000000.0 / 1200.0 / 2) // 416.67 µs gerundet auf 417 µs ( 833.33 µs / 2)
#define DALI_RMT_RESOLUTION_HZ 1000000   // 1 MHz (1 Mikrosekunde pro Tick)
#define DALI_USTONS(x) ((uint)(x * 1000))
#define DALI_TE_TO_MS(x) ((uint)(x * DALI_TE / 1000))
#define DALI_TE_TO_US(x) ((uint)(x * DALI_TE))
#define DALI_RX_BITS 128 // 64 + 1 Start + 2 Stopbits - 128 is next valid value

// Core selection constants for ESP32
#define CORE_0    (0)
#define CORE_1    (1)
#ifndef RMT_CORE
#define RMT_CORE  CORE_1   // Default: Force RMT to run on core 1
#endif

// Conversion from TE (in microseconds) to the number of RMT symbols
#define DALI_TE_TO_RMT(x) ((uint16_t)((x) * (DALI_RMT_RESOLUTION_HZ / 1000000.0)))
