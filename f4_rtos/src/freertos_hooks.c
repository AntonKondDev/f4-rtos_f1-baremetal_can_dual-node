#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "freertos_hooks.h"

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

void vAssertCalled(const char *file, int line)
{
    (void)file;
    (void)line;
    taskDISABLE_INTERRUPTS();
    for (;;) {}
}

void vApplicationTickHook(void)
{
    HAL_IncTick();
}