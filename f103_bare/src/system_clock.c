#include "stm32f1xx_hal.h"
#include "system_clock.h"

// Blue Pill: HSE = 8 MHz → PLL = x9 → SYSCLK = 72 MHz
// AHB  = 72 MHz
// APB1 = 36 MHz  (важно для CAN1)
// APB2 = 72 MHz
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    // Выбираем, какие источники тактирования будем конфигурировать
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;         // Включаем внешний кварц (HSE ON)
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;// Деление на 1 → PLL получает ровно частоту HSE
    osc.PLL.PLLState   = RCC_PLL_ON;         // Включаем PLL, чтобы получить высокую частоту
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;  // Источник для PLL = внешний кварц
    osc.PLL.PLLMUL     = RCC_PLL_MUL9;       // Множитель PLL: HSE(8 МГц) * 9 = 72 МГц
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        while (1) { __NOP(); } // критическая ошибка тактирования
    }

    // Дериватики шин и источник SYSCLK
    clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                         RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; // SYSCLK = 72
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;         // HCLK  = 72
    clk.APB1CLKDivider = RCC_HCLK_DIV2;           // PCLK1 = 36  (CAN на этой шине)
    clk.APB2CLKDivider = RCC_HCLK_DIV1;           // PCLK2 = 72
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) {
        while (1) { __NOP(); }
    }

    // SysTick = 1 мс
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}