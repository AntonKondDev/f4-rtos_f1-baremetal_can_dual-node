#pragma once

#include "stm32f4xx_hal.h"

extern CAN_HandleTypeDef hcan2;
/* Инициализация CAN2 в NORMAL, 500 кбит/с + фильтр "принять всё" */
HAL_StatusTypeDef CAN2_Init_500k_NormalMode(void);
