#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "main.h"

typedef enum
{
    DHT11_OK = 0,
    DHT11_TIMEOUT,
    DHT11_CHECKSUM_ERROR
} DHT11_Result;

void DHT11_Init(void);
DHT11_Result DHT11_Read(float *temperature, float *humidity);

#endif