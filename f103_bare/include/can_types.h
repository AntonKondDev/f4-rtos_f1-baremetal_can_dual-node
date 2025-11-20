#pragma once
#include <stdint.h>

/* Универсальный контейнер для кадра CAN */
typedef struct {
    uint32_t id;      // 11-битный StdId (в младших битах)
    uint8_t  dlc;     // длина полезных данных 0..8
    uint8_t  data[8]; // полезные данные
} CanFrame_t;