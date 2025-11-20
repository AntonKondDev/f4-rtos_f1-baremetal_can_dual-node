#include "stm32f1xx_hal.h"
#include "system_clock.h"
#include "app_logic.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    App_Init();

    while (1) {
        App_Poll();         // логика
        // маленькая пауза не обязательна, но уменьшает нагрузку на шину AHB
        HAL_Delay(1);
    }
}