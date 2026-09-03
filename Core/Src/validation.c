#include "validation.h"
#include <stddef.h>

bool SensorData_Validate(const SensorData *data)
{
    if (data == NULL)
    {
        return false;
    }

    /*
     * DHT11 temperature measurement range:
     * approximately 0 to 50 degree Celsius.
     */
    if ((data->temperature < 0.0f) ||
        (data->temperature > 50.0f))
    {
        return false;
    }

    /*
     * Physical relative humidity range.
     */
    if ((data->humidity < 0.0f) ||
        (data->humidity > 100.0f))
    {
        return false;
    }

    /*
     * BMP280 pressure measurement range.
     */
    if ((data->pressure < 300.0f) ||
        (data->pressure > 1100.0f))
    {
        return false;
    }

    return true;
}