#pragma once
#ifdef ARDUINO_ARCH_ESP32
#include "Dali/defines.h"
#include "driver/rmt_types.h"

// DALI-Symbols for RMT transmission and reception
static const rmt_symbol_word_t DALI_SYMBOL_ONE = {
    .duration0 = DALI_TE_TO_RMT(DALI_TE),
    .level0 = 1,
    .duration1 = DALI_TE_TO_RMT(DALI_TE),
    .level1 = 0,
};

static const rmt_symbol_word_t DALI_SYMBOL_ZERO = {
    .duration0 = DALI_TE_TO_RMT(DALI_TE),
    .level0 = 0,
    .duration1 = DALI_TE_TO_RMT(DALI_TE),
    .level1 = 1,
};

static const rmt_symbol_word_t DALI_SYMBOL_STOP = {
    .duration0 = DALI_TE_TO_RMT(DALI_TE),
    .level0 = 0,
    .duration1 = DALI_TE_TO_RMT(DALI_TE),
    .level1 = 0,
};
#endif
