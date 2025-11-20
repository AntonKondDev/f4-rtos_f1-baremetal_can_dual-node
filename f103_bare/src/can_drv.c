#include "stm32f1xx_hal.h"
#include "can_drv.h"

/* Глобальный дескриптор CAN — виден в других модулях через extern */
CAN_HandleTypeDef hcan1;

/* Внутренняя настройка GPIO для CAN1: PA12=TX (AF PP), PA11=RX */
static void CAN1_GPIO_Init(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // На Blue Pill CAN1 можно вывести на PA11/PA12 remap'ом:
    __HAL_AFIO_REMAP_CAN1_1(); // PA11=RX, PA12=TX

    GPIO_InitTypeDef g = {0};
    g.Speed = GPIO_SPEED_FREQ_HIGH;

    g.Pin  = GPIO_PIN_12;        // TX
    g.Mode = GPIO_MODE_AF_PP;
    HAL_GPIO_Init(GPIOA, &g);

    g.Pin  = GPIO_PIN_11;        // RX
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_NOPULL;        // внешние уровни задаёт трансивер
    HAL_GPIO_Init(GPIOA, &g);
}

/* Настройка CAN:
 *  - тактирование
 *  - GPIO
 *  - настройки биттайминга на 500 кбит/с
 *  - фильтр “принять всё” в FIFO0
 *  - HAL_CAN_Start()
 */
HAL_StatusTypeDef CAN1_Init_500k_Normal(void)
{
    __HAL_RCC_CAN1_CLK_ENABLE();
    CAN1_GPIO_Init();

    hcan1.Instance                    = CAN1;
    hcan1.Init.Mode                   = CAN_MODE_NORMAL;
    hcan1.Init.Prescaler              = 6;            // 36 МГц / 6 = 6 МГц
    hcan1.Init.SyncJumpWidth          = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1               = CAN_BS1_9TQ;  // 1 + 9 + 2 = 12 TQ  → 6e6/12 = 500 кбит/с
    hcan1.Init.TimeSeg2               = CAN_BS2_2TQ;
    hcan1.Init.AutoBusOff             = ENABLE;       //автовыход из Bus-off
    hcan1.Init.AutoWakeUp             = DISABLE;
    hcan1.Init.AutoRetransmission     = ENABLE;
    hcan1.Init.ReceiveFifoLocked      = DISABLE;
    hcan1.Init.TransmitFifoPriority   = DISABLE;

    HAL_StatusTypeDef r = HAL_CAN_Init(&hcan1);
    if (r != HAL_OK) return r;

    // Фильтр “принять всё” → FIFO0
    CAN_FilterTypeDef f = {0};
    f.FilterBank           = 0;
    f.FilterMode           = CAN_FILTERMODE_IDMASK;
    f.FilterScale          = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh         = 0x0000;
    f.FilterIdLow          = 0x0000;
    f.FilterMaskIdHigh     = 0x0000;
    f.FilterMaskIdLow      = 0x0000;
    f.FilterFIFOAssignment = CAN_RX_FIFO0;
    f.FilterActivation     = ENABLE;

    r = HAL_CAN_ConfigFilter(&hcan1, &f);
    if (r != HAL_OK) return r;

    CAN1->FMR &= ~CAN_FMR_FINIT; (void)CAN1->FMR;

    return HAL_CAN_Start(&hcan1);
}

/* Отправка кадра:
 *    — ждём свободный mailbox
 *    — пишем кадр
 *    — ждём до ~2 мс, чтобы mailbox “ушёл” */
HAL_StatusTypeDef CAN1_Send(const CanFrame_t *f)
{
    CAN_TxHeaderTypeDef tx = {0};
    tx.IDE  = CAN_ID_STD;
    tx.StdId= f->id & 0x7FF;
    tx.RTR  = CAN_RTR_DATA;
    tx.DLC  = f->dlc;

    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) { 
      /* ждём свободный mailbox */ 
    }

    uint32_t mb;
    HAL_StatusTypeDef st = HAL_CAN_AddTxMessage(&hcan1, &tx, (uint8_t*)f->data, &mb);
    if (st != HAL_OK) return st;

    // короткое ожидание, чтобы не забивать три mailbox подряд
    uint32_t t0 = HAL_GetTick();
    while (HAL_CAN_IsTxMessagePending(&hcan1, mb) && (HAL_GetTick() - t0 < 2)) { 
      /* wait */ 
    }

    return HAL_OK;
}

/* Есть ли в FIFO0 кадры? */
int CAN1_RxAvail(void)
{
    return (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0);
}

/* Считать один кадр из FIFO0 */
HAL_StatusTypeDef CAN1_Recv(CanFrame_t *f)
{
    CAN_RxHeaderTypeDef rx;
    uint8_t data[8];

    HAL_StatusTypeDef st = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx, data);
    if (st != HAL_OK) return st;

    f->id  = (rx.IDE == CAN_ID_STD) ? rx.StdId : 0;
    f->dlc = rx.DLC;
    for (uint8_t i = 0; i < rx.DLC; ++i) f->data[i] = data[i];

    return HAL_OK;
}
