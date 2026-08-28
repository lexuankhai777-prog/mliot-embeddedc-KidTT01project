#include "evaluate.h"


/* ==========================================================
 * TEMPERATURE THRESHOLDS (degree Celsius)
 * ========================================================== */

#define TEMP_ALARM_LOW       20.0f
#define TEMP_NORMAL_LOW      22.0f
#define TEMP_NORMAL_HIGH     34.0f
#define TEMP_ALARM_HIGH      38.0f


/* ==========================================================
 * HUMIDITY THRESHOLDS (%RH)
 * ========================================================== */

#define HUM_ALARM_LOW        35.0f
#define HUM_NORMAL_LOW       45.0f
#define HUM_NORMAL_HIGH      70.0f
#define HUM_ALARM_HIGH       80.0f


/* ==========================================================
 * PRESSURE THRESHOLDS (hPa)
 * ========================================================== */

#define PRESS_ALARM_LOW      995.0f
#define PRESS_NORMAL_LOW     1000.0f
#define PRESS_NORMAL_HIGH    1015.0f
#define PRESS_ALARM_HIGH     1020.0f


SystemStatus EvaluateState(const SensorData *data)
{
    float temperature = data->temperature;
    float humidity    = data->humidity;
    float pressure    = data->pressure;


    /* ======================================================
     * ALARM CHECK
     *
     * Any parameter in an ALARM range makes the whole
     * environmental status ALARM.
     * ====================================================== */

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


    /* ======================================================
     * WARNING CHECK
     *
     * At this point no parameter is in ALARM.
     * Any parameter outside the NORMAL range becomes WARNING.
     * ====================================================== */

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


    /* ======================================================
     * NORMAL
     * ====================================================== */

    return STATUS_NORMAL;
}