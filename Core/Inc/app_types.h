#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    float temperature;
    float humidity;
    float pressure;
    bool valid;
} SensorData;

typedef enum
{
    STATUS_NORMAL = 0,
    STATUS_WARNING,
    STATUS_ALARM
} SystemStatus;

typedef enum
{
    FSM_IDLE = 0,
    FSM_READ_SENSOR,
    FSM_VALIDATE,
    FSM_EVALUATE,
    FSM_UPDATE_OUTPUT,
    FSM_ERROR,
    FSM_SENSOR_FAULT
} FsmState;

typedef enum
{
    ERR_NONE             = 0,

    ERR_DHT11_TIMEOUT    = (1U << 0),
    ERR_DHT11_CHECKSUM   = (1U << 1),

    ERR_BMP280_TIMEOUT   = (1U << 2),
    ERR_BMP280_COMM      = (1U << 3),

    ERR_DATA_INVALID     = (1U << 4)

} ErrorFlag;

#endif