#ifndef I2C_LOWLEVEL_H
#define I2C_LOWLEVEL_H

#include "main.h"
#include <stdint.h>

typedef enum
{
    I2C_RESULT_OK = 0,
    I2C_RESULT_TIMEOUT,
    I2C_RESULT_NACK,
    I2C_RESULT_ERROR
} I2C_Result;

/*
 * address = 7-bit address.
 *
 * Example:
 * BMP280 = 0x76 or 0x77
 * LCD    = 0x27
 */

I2C_Result I2C_Write(uint8_t address,
                    const uint8_t *data,
                    uint16_t length);

I2C_Result I2C_Read(uint8_t address,
                   uint8_t *data,
                   uint16_t length);

I2C_Result I2C_MemRead(uint8_t address,
                       uint8_t reg,
                       uint8_t *data,
                       uint16_t length);

I2C_Result I2C_MemWrite(uint8_t address,
                        uint8_t reg,
                        const uint8_t *data,
                        uint16_t length);

#endif