#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include "TaskLedFast.h"
#include "busywait.h"

void TaskLedFast(void *arg)
{
    (void)arg;
    for (;;)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        busy_ms(2);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}