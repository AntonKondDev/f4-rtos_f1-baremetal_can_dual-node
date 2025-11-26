# Two-Node CAN Communication  
STM32F4 (FreeRTOS) ↔ STM32F103 (Bare-metal)  

## Проект демонстрирует:  
- Реальный двунаправленный обмен по CAN (500 кбит/с) между STM32F407 (FreeRTOS) и STM32F103 (bare-metal).
- Использование STM32 HAL, настройки bit timing под разные APB1.
- Работу с FreeRTOS: задачи, очереди, разделение драйвера / логики.
- Приём по CAN на F1 через прерывания (FIFO0).
- Диагностику: логический анализатор + осциллограф (CANH/CANL). 

## Скриншоты и видео
- [Данные с логического анализатора](./docs/LogicAnalyzer.png)
На скриншоте показан обмен PING (0x100) → PONG (0x101) между STM32F4 и STM32F103. Каналы логического анализатора подключены к входам CRX обоих трансиверов, поэтому каждый кадр декодируется одновременно на двух каналах. Это подтверждает, что обе ноды сидят на общей шине CAN и корректно принимают кадры без ошибок.
- [Видео-демонстрация](https://youtu.be/b8SUo0xdj2Y)
- [Фото F103](./docs/IMG1.jpg)
- [Фото F407](./docs/IMG2.jpg)
- [Осциллограмма](./docs/oscillogram.jpg) одного CAN-кадра (500 kbit/s) на линии CANH/CANL. Измерение выполнено однополярным щупом ×10 на бюджетном DSO2C10(+собрано на макетке), поэтому видны звонки и шум, но уровни и длительности соответствуют ожиданиям по CAN-спецификации. Верхний сигнал — **CANH** (на выходе трансивера со стороны STM32F4). Нижний сигнал — **CANL** (та же линия, тот же кадр, снятый отдельно). Хорошо видно: доминантные уровни (CANH ≈ 3.5 V, CANL ≈ 1.5 V), recessive-состояние (обе линии около 2.5 V), дифференциальный характер сигнала (амплитуда ~2 V между CANH и CANL).

# 1. Соединения  
Проект использует два CAN-трансивера SN65HVD230.    

| Сигнал | STM32F103 | STM32F407 |  
|--------|-----------|-----------|  
| CAN_TX | PA12      | PB13 (CAN2_TX) |  
| CAN_RX | PA11      | PB12 (CAN2_RX) |  
| CANH   | transceiver → transceiver |  
| CANL   | transceiver → transceiver |  
| GND    | общий GND |  общий GND | 

## Терминация  
- Сопротивление между CANH–CANL должно быть 60–120 Ω (обычно 120 Ω на каждом конце линии).  
- Если трансиверы уже имеют встроенные терминаторы, то внешний резистор может не понадобиться.  
---

# 2. Bit-timing (500 кбит/с)  
### STM32F4 (APB1 = 42 МГц)  
- `Prescaler = 7`  
- `BS1 = 9TQ`  
- `BS2 = 2TQ`  
- `SJW = 1TQ`

Проверка:
- 42 MHz / 7 = 6 MHz  
- 6 MHz / (1 + 9 + 2) = 6 MHz / 12 = **500 кбит/с**

### STM32F103 (APB1 = 36 МГц)  
- `Prescaler = 6`  
- `BS1 = 9TQ`  
- `BS2 = 2TQ`  
- `SJW = 1TQ`

Проверка:
- 36 MHz / 6 = 6 MHz  
- 6 MHz / (1 + 9 + 2) = 6 MHz / 12 = **500 кбит/с**

---

# 3. Логика обмена  
### 3.1. F4 → F1 (PING)  
Каждые 500 мс F4 отправляет кадр:  
- **ID:** `0x100`   //standard ID  
- **DLC:** `1`      //код длины данных от 0 до 8  
- **DATA:** `[seq]` //где `seq` — счётчик (uint8_t, инкрементируется каждый раз)  
### 3.2. F1 → F4 (PONG)  
F1 в ответ на PING формирует кадр:  
- **ID:** `0x101`  
- **DLC:** `1`  
- **DATA:** `[seq]` — тот же байт, что пришёл в PING.  
### 3.3. Индикация  

| Плата | LED  | Событие                         |
|-------|------|---------------------------------|
| F4    | PD13 | Успешная передача PING (0x100)  |
| F4    | PD12 | Успешный приём PONG (0x101)     |
| F1    | PC13 | Приём PING (0x100)              |

---
# 4. Структура репозитория  
## STM32F4 (FreeRTOS)  
```
f4-rtos/
├── src/
│   ├── main.c             # Точка входа, запуск FreeRTOS и задач
│   ├── system_clock.c     # Настройка PLL: SYSCLK = 168 МГц, APB1 = 42 МГц
│   ├── gpio.c             # Инициализация LEDs
│   ├── can_drv.c          # Драйвер CAN2 (инициализация, отправка/приём)
│   ├── can_tasks.c        # Задачи FreeRTOS: TaskCanTx / TaskCanRx
│   ├── app_logic_task.c   # Логика PING/PONG на стороне F4
│   └── freertos_hooks.c   # Хуки FreeRTOS: Tick, Idle, MallocFail и др.
├── include/
│   ├── can_drv.h
│   ├── can_tasks.h
│   ├── app_logic_task.h
│   ├── gpio.h
│   ├── system_clock.h
│   └── FreeRTOSConfig.h
└── platformio.ini         # Конфигурация PlatformIO для F4-таргета
```
## STM32F103 (bare-metal HAL)  
```
f103_bare/
├── src/
│   ├── main.c             # Точка входа, основной цикл и App_Poll()
│   ├── system_clock.c     # PLL: SYSCLK = 72 МГц, APB1 = 36 МГц, SysTick
│   ├── gpio.c             # Инициализация LED PC13
│   ├── can_drv.c          # Инициализация CAN1, IRQ, отправка/приём
│   └── app_logic.c        # Логика приёма PING и ответа PONG
├── include/
│   ├── gpio.h
│   ├── can_drv.h
│   ├── app_logic.h
│   └── system_clock.h
└── platformio.ini         # Конфигурация PlatformIO для F103-таргета
```
---

# 5. Описание задач FreeRTOS (F4)  
## 5.1. TaskCanTx  
- Блокируется на очереди gCanTxQ.  
- При наличии кадра в очереди формирует CAN_TxHeaderTypeDef и вызывает HAL_CAN_AddTxMessage.  
- При успешной отправке кратко мигает PD13 (TX OK).  
- В случае ошибки(NACK, bus-off и т.п.) TX мигает PD14 другим паттерном (например, дольше).
## 5.2. TaskCanRx  
- Периодически проверяет состояние FIFO0 (HAL_CAN_GetRxFifoFillLevel).  
- При наличии кадров читает их через HAL_CAN_GetRxMessage.  
- Преобразует в CanFrame_t и кладёт в очередь gCanRxQ для верхнего уровня (TaskApp).  
- При приёме кадра может кратко мигать PD13 или отдельным LED по желанию.  
## 5.3. TaskApp  
- Высокоуровневая логика:  
- Каждые 500 мс формирует PING-кадр 0x100 и отправляет его в очередь gCanTxQ.  
- Параллельно читает очередь gCanRxQ: если приходит кадр с ID 0x101 и dlc > 0, кратко мигает PD12 (подтверждение успешного ответа от F1).  
- Между итерациями делает vTaskDelay(pdMS_TO_TICKS(10)), чтобы не крутиться в холостую и отдавать CPU другим задачам.  
---

# 6. CAN драйверы  
## 6.1. STM32F4 (CAN2, can_drv.c). Основные моменты:
- Настройка пинов PB12/PB13 в AF9 для CAN2 (без внутренних подтяжек).
- Включение тактирования CAN1/CAN2, общий аппаратный сброс модулей.
- Инициализация CAN2 на 500 кбит/с (Prescaler/BS1/BS2/SJW как в разделе 2).
- **Фильтры для CAN2**
  - CAN2 делит фильтры с CAN1, поэтому:
    - включается тактирование CAN1;
    - в структуре `CAN_FilterTypeDef` указывается:
      - `FilterBank           = 14` (первый банк, отданный под CAN2),
      - `SlaveStartFilterBank = 14` (банки 0..13 → CAN1, 14..→ CAN2),
      - `FilterMode           = CAN_FILTERMODE_IDMASK`,
      - `FilterScale          = CAN_FILTERSCALE_32BIT`,
      - `FilterIdHigh/Low     = 0`,
      - `FilterMaskIdHigh/Low = 0` (принять все ID),
      - `FilterFIFOAssignment = CAN_RX_FIFO0`,
      - `FilterActivation     = ENABLE`.
  Маска 0x0000 означает «принять все идентификаторы» в FIFO0. Это даёт простой фильтр: «принять всё в FIFO0 для CAN2».
- Очистка флагов ошибок (ESR, ERRI, FOV0 и т.д.).
- Ключевые функции на стороне F4:
  - `HAL_StatusTypeDef CAN2_Init_500k_NormalMode(void)` // инициализация CAN2 на 500 кбит/с.
  - `void TaskCanTx(void *arg)` // RTOS-таск, который берёт кадры из очереди `gCanTxQ` и отправляет их по CAN2.
  - `void TaskCanRx(void *arg)` // RTOS-таск, который принимает кадры из FIFO0 CAN2 и складывает их в очередь `gCanRxQ`.

## 6.2. STM32F103 (CAN1). Основные шаги:
- Настройка PA11 (RX) и PA12 (TX) под CAN1.
- Включение APB1 тактирования CAN1.
- Bit-timing 500 кбит/с (см. 2.2).
- Настройка фильтра “принять все” в FIFO0 (f.FilterBank = 0).
- Включение CAN и прерываний:
HAL_CAN_Start(&hcan1);
HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1, 0);
HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

- Обработчик прерывания FIFO0:
```
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan1);   // делегируем обработку HAL'у
}
```
- Callback при появлении сообщения в FIFO0:
```
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx;
    uint8_t data[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx, data) == HAL_OK) {
        // разбор ID/DLC, индикация, формирование PONG-кадра и отправка
    }
}
```
---

# 7. Сборка и прошивка  

### 7.1. Необходимые инструменты  
**Минимальный набор:**
- [Visual Studio Code](https://code.visualstudio.com/)
- Расширение **PlatformIO IDE** для VS Code
- **Git for Windows** — даёт:
  - консоль `git`,
  - удобный терминал **Git Bash** с POSIX-командами (`ls`, `rm`, `make` и т.п.).

**На Linux/macOS достаточно:**
- системного терминала,
- установленного `git`,
- PlatformIO (`pip install platformio` или установка через VS Code).

**Hardware:**
- Платы **STM32F407 Discovery** и **STM32F103C8 (Blue Pill)**.
- Программатор **ST-LINK** (для F1, F4 уже имеет встроенный ST-LINK).
- Внешний блок питания **5 В** (желательно питать обе платы от общего источника).
- Понижающий DC-DC преобразователь (5в - 3.3в), для питания трансиверов.
- Один или два резистора **120 Ω** для терминаторов линии CAN (если не встроены в трансивер).

---  

### 7.2. Сборка и прошивка (PlatformIO)  
В терминале VS Code (на Windows удобно использовать Git Bash):  

# STM32F407 (FreeRTOS)  
```  
cd f4-rtos          # выбор каталога проекта  
pio run             # сборка  
pio run -t upload   # прошивка через встроенный ST-LINK  
```  
# STM32F103 (bare-metal)  
```  
cd f103_bare        # выбор каталога проекта  
pio run             # сборка  
pio run -t upload   # прошивка через ST-LINK  
```  

# 8. Проверка работы после прошивки  
1. Убедитесь, что обе платы запитаны от одного источника 5 В  
(лучше обе платы от внешнего БП, чтобы избежать петель и перетоков).  
2. Проверьте индикацию:  
На F4: PD13 мигает при отправке PING (0x100). PD12 мигает при получении PONG (0x101) от F1.  
На F1: PC13 мигает при приёме PING (0x100).  
3. Если PD13 мигает, а PC13 — нет, значит F103 не принимает кадры. Проверьте:  
- настройки CAN bit-timing на обеих платах;  
- фильтры (режим «принять всё» в FIFO0);  
- фактическую проводку линии (CANH/CANL, GND, питание трансиверов).  

# 9. Проверка физического подключения  
- Общий GND для F4, F1 и обоих трансиверов (без общей земли шина CAN работать не будет).  
- Линии CANH → CANH, CANL → CANL между трансиверами.  
- Сопротивление между CANH–CANL без питания: примерно 60–120 Ω (с учётом встроенных/внешних терминаторов).  
- Пин RS на SN65HVD230 обычно подтягивается к GND (нормальный режим работы).  
- Пины CAN-контроллера MCU: F1: PA11/PA12, F4: PB12/PB13, настроены в режим AF-CAN без внутренних подтяжек.  