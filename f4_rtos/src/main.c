#include "gpio.h"
#include "system_clock.h"
#include "tasks.h"
#include "rtos.h"
#include "can_min.h"
#include "busywait.h"

int main(void)
{
  HAL_Init();
    SystemClock_Config();     
    GPIO_Init();
    CAN_Min_Start();
    //Tasks_Init();
    vTaskStartScheduler();
}