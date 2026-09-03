#include "evaluate.h"

/* ==========================================================
 * TEMPERATURE THRESHOLDS
 * Unit: degree Celsius
 * ========================================================== */

#define TEMP_ALARM_LOW       15.0f
#define TEMP_NORMAL_LOW      20.0f
#define TEMP_NORMAL_HIGH     30.0f
#define TEMP_ALARM_HIGH      35.0f


/* ==========================================================
 * HUMIDITY THRESHOLDS
 * Unit: %RH
 * ========================================================== */

#define HUM_ALARM_LOW        30.0f
#define HUM_NORMAL_LOW       40.0f
#define HUM_NORMAL_HIGH      60.0f
#define HUM_ALARM_HIGH       70.0f


/* ==========================================================
 * PRESSURE THRESHOLDS
 * Unit: hPa
 * ========================================================== */

#define PRESS_ALARM_LOW      985.0f
#define PRESS_NORMAL_LOW     995.0f
#define PRESS_NORMAL_HIGH    1020.0f
#define PRESS_ALARM_HIGH     1030.0f


SystemStatus EvaluateState(const SensorData *data)
{
    float temperature = data->temperature;
    float humidity    = data->humidity;
    float pressure    = data->pressure;

    /*
     * ALARM:
     * worst-case wins.
     */
    if ((temperature < TEMP_ALARM_LOW) ||
        (temperature >= TEMP_ALARM_HIGH))
    {
        return STATUS_ALARM;
    }

    if ((humidity < HUM_ALARM_LOW) ||
        (humidity > HUM_ALARM_HIGH))
    {
        return STATUS_ALARM;
    }

    if ((pressure < PRESS_ALARM_LOW) ||
        (pressure > PRESS_ALARM_HIGH))
    {
        return STATUS_ALARM;
    }

    /*
     * WARNING:
     * no parameter is in ALARM,
     * but at least one is outside NORMAL range.
     */
    if ((temperature < TEMP_NORMAL_LOW) ||
        (temperature > TEMP_NORMAL_HIGH))
    {
        return STATUS_WARNING;
    }

    if ((humidity < HUM_NORMAL_LOW) ||
        (humidity > HUM_NORMAL_HIGH))
    {
        return STATUS_WARNING;
    }

    if ((pressure < PRESS_NORMAL_LOW) ||
        (pressure > PRESS_NORMAL_HIGH))
    {
        return STATUS_WARNING;
    }

    return STATUS_NORMAL;
}