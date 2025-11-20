#include "rtos.h"
#include "can_types.h"
#include "can_drv.h" 
#include "can_tasks.h"

QueueHandle_t gCanTxQ = NULL;
QueueHandle_t gCanRxQ = NULL;

/* Берёт кадры из очереди gCanTxQ и отправляет их */
void TaskCanTx(void *arg)
{
    (void)arg;

    for (;;)
    {
        CanFrame_t fr;
        if (xQueueReceive(gCanTxQ, &fr, portMAX_DELAY) == pdPASS)
        {
            CAN_TxHeaderTypeDef tx = {0};
            tx.IDE = CAN_ID_STD;
            tx.StdId = fr.id & 0x7FF;
            tx.RTR = CAN_RTR_DATA;
            tx.DLC = fr.dlc;

            uint32_t mb;
            if (HAL_CAN_AddTxMessage(&hcan2, &tx, (uint8_t*)fr.data, &mb) == HAL_OK)
            {
                /* TX OK: короткая вспышка PD13 */
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(10));
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
            }
            else
            {
                /* Ошибка TX: мигнуть PD14 подольше */
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(60));
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
            }
        }
    }
}

/* Пулинг FIFO0 каждые 5 мс, кладёт принятый кадр в gCanRxQ */
void TaskCanRx(void *arg)
{
    (void)arg;

    CAN_RxHeaderTypeDef rx;
    uint8_t d[8];

    for (;;)
    {
        while (HAL_CAN_GetRxFifoFillLevel(&hcan2, CAN_RX_FIFO0) > 0)
        {
            if (HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &rx, d) == HAL_OK)
            {
                CanFrame_t fr = {0};
                fr.id  = rx.StdId;
                fr.dlc = rx.DLC;
                for (uint8_t i = 0; i < rx.DLC; ++i) fr.data[i] = d[i];

                (void)xQueueSend(gCanRxQ, &fr, 0);

                /* RX OK: короткая вспышка PD12 */
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(5));
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void CAN_Tasks_Init(void)
{
    gCanTxQ = xQueueCreate(16, sizeof(CanFrame_t));
    gCanRxQ = xQueueCreate(16, sizeof(CanFrame_t));

    configASSERT(gCanTxQ != NULL);
    configASSERT(gCanRxQ != NULL);

    xTaskCreate(TaskCanTx, "can_tx", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(TaskCanRx, "can_rx", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
}