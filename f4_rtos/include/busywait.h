#pragma once
#include "stm32f4xx.h"
#include <stdint.h>

static inline void busy_ms(uint32_t ms)
{
    // Быстрая задержка через DWT (точно и без SysTick)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    uint32_t start = DWT->CYCCNT, ticks = (SystemCoreClock/1000u) * ms;
    while ((DWT->CYCCNT - start) < ticks) { __NOP(); }
}
