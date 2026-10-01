#include "sht31.h"

extern I2C_HandleTypeDef hi2c1;

#define SHT31_ADDR (0x45 << 1)
static const uint8_t cmd_measure[2] = {0x24, 0x0B};

/**
 * @brief Read temperature and humidity from SHT31.
 *        Uses HAL I2C driver (blocking).
 */
HAL_StatusTypeDef sht31_read(int *temperature, int *humidity)
{
    uint8_t data[6];

    /* Trigger measurement */
    if (HAL_I2C_Master_Transmit(&hi2c1, SHT31_ADDR, (uint8_t*)cmd_measure, 2, 50) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(15);  // Measurement time

    /* Read 6 bytes: T(2)+CRC, RH(2)+CRC */
    if (HAL_I2C_Master_Receive(&hi2c1, SHT31_ADDR, data, 6, 50) != HAL_OK)
        return HAL_ERROR;

    uint16_t rawT  = (data[0] << 8) | data[1];
    uint16_t rawRH = (data[3] << 8) | data[4];

    float t = -45.0f + 175.0f * ((float)rawT / 65535.0f);
    float h = 100.0f * ((float)rawRH / 65535.0f);

    *temperature = (int)t;
    *humidity    = (int)h;

    return HAL_OK;
}
