#ifndef BMP280_H
#define BMP280_H

#include "main.h"
#include <stdint.h>

typedef enum
{
    BMP280_OK = 0,
    BMP280_TIMEOUT,
    BMP280_COMM_ERROR,
    BMP280_INIT_ERROR
} BMP280_Result;

#define BMP280_ADDRESS_76    0x76U
#define BMP280_ADDRESS_77    0x77U

BMP280_Result BMP280_Init(void);

BMP280_Result BMP280_ReadPressure(float *pressure_hpa);

uint8_t BMP280_GetAddress(void);

#endif