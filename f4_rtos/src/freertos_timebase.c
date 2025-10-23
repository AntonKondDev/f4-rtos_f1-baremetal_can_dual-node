#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

uint32_t HAL_GetTick(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        return (uint32_t)xTaskGetTickCount();

    extern __IO uint32_t uwTick;
    return uwTick;
}
