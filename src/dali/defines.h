#define DALI_TE (1000000.0 / 1200.0 / 2) // 416.67 µs gerundet auf 417 µs ( 833.33 µs / 2)
#define DALI_RMT_RESOLUTION_HZ 1000000   // 1 MHz (1 Mikrosekunde pro Tick)
#define DALI_USTONS(x) ((uint)(x * 1000))
#define DALI_TE_TO_MS(x) ((uint)(x * DALI_TE / 1000))
#define DALI_TE_TO_US(x) ((uint)(x * DALI_TE))
#define DALI_RX_BITS 128 // 64 + 1 Start + 2 Stopbits - 128 is next valid value

#define DALI_THRESHOLD_1TE_LOW ((uint)(DALI_TE * 0.8))
#define DALI_THRESHOLD_1TE_HIGH ((uint)(DALI_TE * 1.2))
#define DALI_THRESHOLD_2TE_LOW ((uint)(2 * DALI_THRESHOLD_1TE_LOW))
#define DALI_THRESHOLD_2TE_HIGH ((uint)(2 * DALI_THRESHOLD_1TE_HIGH))