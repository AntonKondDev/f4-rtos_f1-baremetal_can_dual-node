#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include "TaskLedSlow.h"
#include "busywait.h"

void TaskLedSlow(void *arg)
{
    (void)arg;
    for (;;)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        busy_ms(2);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}