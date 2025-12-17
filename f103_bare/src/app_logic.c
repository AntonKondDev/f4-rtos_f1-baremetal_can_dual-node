#include "stm32f1xx_hal.h"
#include "can_drv.h"
#include "gpio.h"
#include "app_logic.h"
#include "can_types.h"

volatile uint8_t led_flag = 0;

void App_Init(void)
{
    GPIO_Init();    // PC13 как LED
    if (CAN1_Init_500k_Normal() != HAL_OK) {
        while (1) {
            LED_Blink(200); // аварийная индикация
        }
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx;
    uint8_t data[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx, data) != HAL_OK) {
        return;
    }

    CanFrame_t f = {0};
    f.id  = (rx.IDE == CAN_ID_STD) ? rx.StdId : rx.ExtId;
    f.dlc = rx.DLC;
    for (uint8_t i = 0; i < f.dlc && i < 8; ++i) {
        f.data[i] = data[i];
    }

    // Логика уровня приложения: PING → PONG → LED_Blink
    if (f.id == 0x100 && f.dlc > 0) {
        CanFrame_t pong = {0};
        pong.id     = 0x101;
        pong.dlc    = 1;
        pong.data[0]= f.data[0];

        (void)CAN1_Send(&pong);
    }
    
    led_flag = 1; 
}

void App_Process(void)
{
  if (led_flag) {
      led_flag = 0;
      LED_Blink(10);
  }
}