#pragma once
#include "stm32f1xx_hal.h"
#include "can_types.h"

/* Глобальный дескриптор — создаётся в can_drv.c */
extern CAN_HandleTypeDef hcan1;

/* Инициализация CAN1 на 500 кбит/с, фильтр “принять всё”, режим NORMAL */
HAL_StatusTypeDef CAN1_Init_500k_Normal(void);

/* Отправка кадра */
HAL_StatusTypeDef CAN1_Send(const CanFrame_t *f);

/* Есть ли кадры в FIFO0 (0 — нет, >0 — есть) */
int CAN1_RxAvail(void);

/* Считать кадр из FIFO0 (HAL_OK при успехе) */
HAL_StatusTypeDef CAN1_Recv(CanFrame_t *f);