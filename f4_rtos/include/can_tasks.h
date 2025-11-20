#pragma once
#include "FreeRTOS.h"
#include "queue.h"

/* Глобальные объекты очередей */
extern QueueHandle_t gCanTxQ;   // на отправку
extern QueueHandle_t gCanRxQ;   // принятые кадры
void CAN_Tasks_Init(void);