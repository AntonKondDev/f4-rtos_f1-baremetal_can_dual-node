#include "gpio.h"

void GPIO_Init(void){
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_13;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &g);
    LED_Off(); // PC13 высокий — светодиод на BluePill погашен
}

void LED_On(void){  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); }
void LED_Off(void){ HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);   }
// обёртка над PC13 (on-board LED) для визуальной индикации RX
void LED_Blink(uint32_t ms)
{
  LED_On();
  HAL_Delay(ms);
  LED_Off();
  HAL_Delay(ms);
}