#pragma once
#include <stdint.h>

/* Простой тип кадра для очередей */
typedef struct {
    uint32_t id;     // std 11-bit
    uint8_t  dlc;    // 0..8
    uint8_t  data[8];
} CanFrame_t;

// Очереди
#define CAN_TX_QUEUE_LEN   16
#define CAN_RX_QUEUE_LEN   16

// ID/команды верхнего уровня
#define ID_PING        0x100
#define ID_PONG        0x101
#define ID_CMD_LED     0x200
#define ID_LED_STATUS  0x201
