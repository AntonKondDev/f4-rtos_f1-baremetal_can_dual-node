#include "rtos.h"
#include "gpio.h"
#include "can_drv.h"
CAN_HandleTypeDef hcan2;

/* CAN2: PB12 = RX, PB13 = TX, AF9 */
static void CAN2_GPIO_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};

    g.Pin       = GPIO_PIN_13;
    g.Mode      = GPIO_MODE_AF_PP;
    g.Pull      = GPIO_NOPULL;
    g.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &g);

    g.Pin       = GPIO_PIN_12;
    g.Mode      = GPIO_MODE_AF_PP;
    g.Pull      = GPIO_NOPULL;
    g.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &g);
}

/* -------- CAN2 @ 500 kbit/s, NORMAL -------- */
HAL_StatusTypeDef CAN2_Init_500k_NormalMode(void)
{
    /* Включаем оба тактирования (CAN2 делит фильтры с CAN1) */
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_CAN2_CLK_ENABLE();

    CAN2_GPIO_Init();

    /* APB1=42MHz: 42/6=7MHz; 7MHz/14TQ ≈ 500 kбит/с */
    hcan2.Instance = CAN2;
    hcan2.Init.Prescaler           = 7;
    hcan2.Init.Mode                = CAN_MODE_NORMAL;
    hcan2.Init.SyncJumpWidth       = CAN_SJW_1TQ;
    hcan2.Init.TimeSeg1            = CAN_BS1_9TQ;
    hcan2.Init.TimeSeg2            = CAN_BS2_2TQ;
    hcan2.Init.AutoBusOff          = DISABLE;
    hcan2.Init.AutoRetransmission  = ENABLE;
    hcan2.Init.AutoWakeUp          = DISABLE;
    hcan2.Init.ReceiveFifoLocked   = DISABLE;
    hcan2.Init.TransmitFifoPriority= DISABLE;

    HAL_StatusTypeDef st = HAL_CAN_Init(&hcan2);
    if (st != HAL_OK) return st;

    /* Фильтр: принять всё в FIFO0; банки 14.. на CAN2 */
    CAN_FilterTypeDef f = {0};
    f.FilterBank           = 14;
    f.SlaveStartFilterBank = 14;
    f.FilterMode           = CAN_FILTERMODE_IDMASK;
    f.FilterScale          = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh         = 0x0000;
    f.FilterIdLow          = 0x0000;
    f.FilterMaskIdHigh     = 0x0000;
    f.FilterMaskIdLow      = 0x0000;
    f.FilterFIFOAssignment = CAN_RX_FIFO0;
    f.FilterActivation     = ENABLE;

    st = HAL_CAN_ConfigFilter(&hcan2, &f);
    if (st != HAL_OK) return st;

    return HAL_CAN_Start(&hcan2);
}