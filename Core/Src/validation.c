#include "validation.h"

bool SensorData_Validate(const SensorData *data)
{
    if (data == NULL)
    {
        return false;
    }

    if (data->humidity < 0.0f || data->humidity > 100.0f)
    {
        return false;
    }

    if (data->pressure <= 0.0f)
    {
        return false;
    }

    return true;
}