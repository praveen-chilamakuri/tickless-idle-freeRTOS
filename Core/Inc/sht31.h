#ifndef SHT31_H
#define SHT31_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef sht31_read(int *temperature, int *humidity);

#endif
