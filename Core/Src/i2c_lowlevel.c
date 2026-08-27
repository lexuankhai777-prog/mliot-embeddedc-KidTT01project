#include "i2c_lowlevel.h"

#include <stddef.h>

extern I2C_HandleTypeDef hi2c1;

#define I2C_LOWLEVEL_TIMEOUT_MS 100U

static I2C_Result I2C_MapHALStatus(HAL_StatusTypeDef status)
{
    if (status == HAL_OK)
    {
        return I2C_RESULT_OK;
    }

    if (status == HAL_TIMEOUT)
    {
        return I2C_RESULT_TIMEOUT;
    }

    if (status == HAL_ERROR)
    {
        uint32_t error = HAL_I2C_GetError(&hi2c1);

        /*
         * AF = Acknowledge Failure.
         * Usually means NACK/no responding device.
         */
        if ((error & HAL_I2C_ERROR_AF) != 0U)
        {
            return I2C_RESULT_NACK;
        }

        return I2C_RESULT_ERROR;
    }

    return I2C_RESULT_ERROR;
}

I2C_Result I2C_Write(uint8_t address,
                    const uint8_t *data,
                    uint16_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return I2C_RESULT_ERROR;
    }

    HAL_StatusTypeDef status =
        HAL_I2C_Master_Transmit(&hi2c1,
                                (uint16_t)(address << 1),
                                (uint8_t *)data,
                                length,
                                I2C_LOWLEVEL_TIMEOUT_MS);

    return I2C_MapHALStatus(status);
}

I2C_Result I2C_Read(uint8_t address,
                   uint8_t *data,
                   uint16_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return I2C_RESULT_ERROR;
    }

    HAL_StatusTypeDef status =
        HAL_I2C_Master_Receive(&hi2c1,
                               (uint16_t)(address << 1),
                               data,
                               length,
                               I2C_LOWLEVEL_TIMEOUT_MS);

    return I2C_MapHALStatus(status);
}

I2C_Result I2C_MemRead(uint8_t address,
                       uint8_t reg,
                       uint8_t *data,
                       uint16_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return I2C_RESULT_ERROR;
    }

    HAL_StatusTypeDef status =
        HAL_I2C_Mem_Read(&hi2c1,
                         (uint16_t)(address << 1),
                         reg,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         length,
                         I2C_LOWLEVEL_TIMEOUT_MS);

    return I2C_MapHALStatus(status);
}

I2C_Result I2C_MemWrite(uint8_t address,
                        uint8_t reg,
                        const uint8_t *data,
                        uint16_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return I2C_RESULT_ERROR;
    }

    HAL_StatusTypeDef status =
        HAL_I2C_Mem_Write(&hi2c1,
                          (uint16_t)(address << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          (uint8_t *)data,
                          length,
                          I2C_LOWLEVEL_TIMEOUT_MS);

    return I2C_MapHALStatus(status);
}