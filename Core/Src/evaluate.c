#include "evaluate.h"

SystemStatus EvaluateState(const SensorData *data)
{
    if (data->temperature >= 45.0f)
    {
        return STATUS_ALARM;
    }

    if (data->temperature >= 35.0f)
    {
        return STATUS_WARNING;
    }

    return STATUS_NORMAL;
}