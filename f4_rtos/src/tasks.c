#include "tasks.h"
#include "TaskLedFast.h"
#include "TaskLedSlow.h"
#include "rtos.h"

static TaskHandle_t hTaskLedFast, hTaskLedkSlow;
void Tasks_Init(void) {
  xTaskCreate(TaskLedFast, "fast", 256, NULL, tskIDLE_PRIORITY + 1, &hTaskLedFast);
  xTaskCreate(TaskLedSlow, "slow", 256, NULL, tskIDLE_PRIORITY + 1, &hTaskLedkSlow);
}
