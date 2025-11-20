#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "system_clock.h"
#include "can_tasks.h"
#include "can_types.h"
#include "app_logic_task.h"

// Периодически шлём PING (0x100) и ждём PONG (0x101) от F1. RX-индикация на PD12
void TaskApp(void *arg)
{
    (void)arg;
    uint8_t seq = 0;
    TickType_t last_tx = xTaskGetTickCount();

    for (;;)
    {
        // Раз в 500 мс отправляем PING (ID = 0x100, 1 байт счётчика)
        if ((xTaskGetTickCount() - last_tx) >= pdMS_TO_TICKS(500)) {
            last_tx = xTaskGetTickCount();

            CanFrame_t fr = {0};
            fr.id  = 0x100;
            fr.dlc = 1;
            fr.data[0] = seq++;

            (void)xQueueSend(gCanTxQ, &fr, 0);
        }

        // Обрабатываем все кадры, которые успели накопиться в RX-очереди
        CanFrame_t rx;
        while (xQueueReceive(gCanRxQ, &rx, 0) == pdPASS) {

            if (rx.id == 0x101 && rx.dlc > 0) {
                // Пришёл PONG от F1
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(20));
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
            }
            // можно добавить обработку других ID
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void App_Task_Init(void)
{
    xTaskCreate(TaskApp , "app" , 512, NULL, tskIDLE_PRIORITY+1, NULL);
}