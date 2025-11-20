#include "rtos.h"
#include "stm32f4xx_hal.h"
#include "can_types.h"
#include "can_drv.h"
#include "can_tasks.h"
#include "app_logic_task.h"
#include "gpio.h"
#include "system_clock.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();        // SYSCLK=168MHz, APB1=42MHz
    GPIO_Init();                 // LEDs, прочие GPIO
    
    CAN2_Init_500k_NormalMode(); // Инициализация CAN2, настройка фильтров
    CAN_Tasks_Init();            // создание очередей и задач TX/RX
    App_Task_Init();             // создание TaskApp (PING/PONG логика)

    vTaskStartScheduler();       // запуск планировщика

    for(;;){}
}
