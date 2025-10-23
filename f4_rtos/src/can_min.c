#include "rtos.h"
#include "stm32f4xx_hal.h"
#include "task.h"
#include "gpio.h"         
#include "can_min.h"
#include "busywait.h"

static CAN_HandleTypeDef hcan2;
static void CAN2_GPIO_Init(void);
static HAL_StatusTypeDef CAN2_Init_Loopback_500k(void);

static void CAN2_GPIO_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};

    // PB12 = CAN2_RX (AF9), Pull-Up обязателен
    g.Pin       = GPIO_PIN_12;
    g.Mode      = GPIO_MODE_AF_PP;
    g.Pull      = GPIO_PULLUP;
    g.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &g);

    // PB13 = CAN2_TX (AF9)
    g.Pin       = GPIO_PIN_13;
    g.Pull      = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &g);
}

// Настраиваем CAN2 на ~500 кбит/с и режим LOOPBACK (шлём сами себе)
static HAL_StatusTypeDef CAN2_Init_Loopback_500k(void)
{
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_CAN2_CLK_ENABLE();
    vTaskDelay(pdMS_TO_TICKS(2));

    // Не «морозим» CAN при паузе отладчика
    DBGMCU->APB1FZ &= ~(DBGMCU_APB1_FZ_DBG_CAN1_STOP | DBGMCU_APB1_FZ_DBG_CAN2_STOP);
    CAN2_GPIO_Init();
    // Настройки HAL (Loopback 500 кбит/с; без SILENT на время диагностики)
    hcan2.Instance                   = CAN2;
    hcan2.Init.Mode                  = CAN_MODE_LOOPBACK;
    hcan2.Init.Prescaler             = 6;              // 42М/6/14TQ ≈ 500 кбит/с
    hcan2.Init.SyncJumpWidth         = CAN_SJW_1TQ;
    hcan2.Init.TimeSeg1              = CAN_BS1_11TQ;
    hcan2.Init.TimeSeg2              = CAN_BS2_2TQ;
    hcan2.Init.AutoBusOff            = DISABLE;
    hcan2.Init.AutoRetransmission    = ENABLE;
    hcan2.Init.AutoWakeUp            = DISABLE;
    hcan2.Init.ReceiveFifoLocked     = DISABLE;
    hcan2.Init.TransmitFifoPriority  = DISABLE;

    HAL_CAN_Init(&hcan2);

    // Фильтр «принять всё» для CAN2: банк >= SlaveStartFilterBank (14)
    CAN_FilterTypeDef f = {0};
    f.FilterBank           = 14;     // первый банк CAN2
    f.SlaveStartFilterBank = 14;     // 0..13 → CAN1, 14.. → CAN2
    f.FilterMode           = CAN_FILTERMODE_IDMASK;
    f.FilterScale          = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh         = 0x0000;  f.FilterIdLow  = 0x0000;
    f.FilterMaskIdHigh     = 0x0000;  f.FilterMaskIdLow = 0x0000;
    f.FilterFIFOAssignment = CAN_RX_FIFO0;
    f.FilterActivation     = ENABLE;

    HAL_CAN_ConfigFilter(&hcan2, &f);

    // Старт HAL (теперь ядро уже вышло из INIT/SLEEP)
    HAL_CAN_Start(&hcan2);

    return HAL_OK;
}
  
  //FreeRTOS-таск: раз в секунду шлём кадр и тут же читаем его обратно
  static void CanPingTask(void *arg) {
      (void)arg;
  
    CAN_TxHeaderTypeDef tx = {0};
    CAN_RxHeaderTypeDef rx = {0};
    uint8_t txd[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    uint8_t rxd[8] = {0};
    
    if (CAN2_Init_Loopback_500k() != HAL_OK) {
        for (;;) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }
    
    for (;;) {
        // подготовим заголовок передачи
        tx.IDE = CAN_ID_STD;               // стандартный 11-битный ID
        tx.StdId = 0x123;                  // любой ID
        tx.RTR = CAN_RTR_DATA;             // Data frame
        tx.DLC = 8;                        // 8 байт данных
        
        // ждем свободный TX mailbox c таймаутом
        TickType_t dl = xTaskGetTickCount() + pdMS_TO_TICKS(20);
        while ((HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0U) &&
               (xTaskGetTickCount() < dl)) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        // Обработка ошибки TX
        uint32_t mb;
        HAL_StatusTypeDef r = HAL_CAN_AddTxMessage(&hcan2, &tx, txd, &mb);
        if (r != HAL_OK) {
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
                    vTaskDelay(pdMS_TO_TICKS(20));
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
                    vTaskDelay(pdMS_TO_TICKS(20));
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
                    continue;
        }
     
        // вспышка TX
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(20));
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

        // Подождём, пока кадр «вернётся» в loopback (простое ожидание с таймаутом)
        TickType_t ddl = xTaskGetTickCount() + pdMS_TO_TICKS(20);
        for (;;) {
            if (HAL_CAN_GetRxFifoFillLevel(&hcan2, CAN_RX_FIFO0) > 0U) {
                if (HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &rx, rxd) == HAL_OK) {
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
                    vTaskDelay(pdMS_TO_TICKS(20));
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
                }
                break;
            }
            if (xTaskGetTickCount() > ddl) {
                HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));   // пауза 1 секунда
    }
  }

void CAN_Min_Start(void) {
    xTaskCreate(CanPingTask, "can_ping", 512, NULL, tskIDLE_PRIORITY+1, NULL); // запустить таск
}
