#include "gpio.h"
#include "stm32f4xx_hal.h"

void GPIO_Init(void)
{
    /* LED: PD12 (RX ok blink), PD13 (TX ok blink), PD14 (ERR) */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};
    g.Pin   = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &g);
}

