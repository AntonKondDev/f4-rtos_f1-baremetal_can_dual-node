#pragma once
#include "stm32f1xx_hal.h"
void GPIO_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Blink(uint32_t ms);