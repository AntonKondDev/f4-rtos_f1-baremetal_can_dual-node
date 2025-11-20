#include "stm32f1xx_hal.h"
#include "can_drv.h"
#include "gpio.h"
#include "app_logic.h"
#include "can_types.h"

void App_Init(void)
{
    GPIO_Init();    // PC13 как LED
    if (CAN1_Init_500k_Normal() != HAL_OK) {
                    // аварийная индикация
        while (1) {
            LED_On();
            HAL_Delay(100);
            LED_Off();
            HAL_Delay(100);
        }
    }
}

void App_Poll(void)
{
                     // Проверим есть ли кадры в FIFO0
    while (CAN1_RxAvail())
    {
        CanFrame_t rx;
        if (CAN1_Recv(&rx) == HAL_OK)
        {
                     // PING 0x100 -> PONG 0x101
            if (rx.id == 0x100 && rx.dlc > 0)
            {
                LED_On();
                HAL_Delay(10);
                LED_Off();

                CanFrame_t tx = {
                    .id   = 0x101,
                    .dlc  = 1,
                    .data = { rx.data[0] }
                };
                (void)CAN1_Send(&tx);
            }
        }
    }
}
