#include "fsm.h"
#include "dht11.h"
#include "bmp280.h"
#include "validation.h"
#include "evaluate.h"
#include "app_types.h"
#include "output.h"
#include "uart_log.h"
#include "main.h"

#define FSM_SAMPLE_PERIOD_MS   2000U
#define FSM_ERROR_THRESHOLD    3U
#define STATUS_CONFIRM_COUNT 3U


static FsmState current_state = FSM_IDLE;

static SensorData current_data;

static SystemStatus current_status = STATUS_NORMAL;
static SystemStatus candidate_status = STATUS_NORMAL;
static uint8_t candidate_count = 0U;

static uint32_t error_flags = ERR_NONE;

static DHT11_Result dht_result;
static BMP280_Result bmp_result;

static bool bmp_initialized = false;

static uint32_t last_sample_tick = 0U;

static uint32_t error_count = 0U;


/* ---------------------------------------------------------- */

static void FSM_ShowErrorOutput(void)
{
    current_data.valid = false;

    candidate_status = current_status;
    candidate_count = 0U;

    Output_ShowError(error_flags);

    Output_UpdateLCD(&current_data,
                     STATUS_NORMAL,
                     error_flags);
}

void FSM_Init(void)
{
    current_state = FSM_IDLE;

    current_data.temperature = 0.0f;
    current_data.humidity    = 0.0f;
    current_data.pressure    = 0.0f;
    current_data.valid       = false;

    current_status = STATUS_NORMAL;
    candidate_status = STATUS_NORMAL;
    candidate_count = 0U;

    error_flags = ERR_NONE;
    error_count = 0U;

    bmp_initialized = false;

    last_sample_tick = HAL_GetTick();

    UART_WriteString(
        "[FSM] Init -> IDLE\r\n");
}


/* ---------------------------------------------------------- */

void FSM_Process(void)
{
    switch (current_state)
    {
        /* ==================================================
         * IDLE
         * ================================================== */
        case FSM_IDLE:
        {
            if ((HAL_GetTick() - last_sample_tick)>= FSM_SAMPLE_PERIOD_MS)
            {
                last_sample_tick = HAL_GetTick();

                UART_WriteString(
                    "[FSM] IDLE -> READ_SENSOR\r\n");

                current_state = FSM_READ_SENSOR;
            }

            break;
        }


        /* ==================================================
         * READ SENSOR
         * ================================================== */
        case FSM_READ_SENSOR:
        {
            error_flags = ERR_NONE;

            current_data.valid = false;


            /*
             * BMP280 initialization / re-initialization.
             *
             * bmp_initialized becomes false whenever
             * BMP280 communication fails.
             */
            if (!bmp_initialized)
            {
                BMP280_Result init_result =
                    BMP280_Init();

                if (init_result == BMP280_OK)
                {
                    bmp_initialized = true;

                    UART_WriteString(
                        "[FSM] BMP280 initialization OK\r\n");
                }
                else
                {
                    if (init_result == BMP280_TIMEOUT)
                    {
                        error_flags |=
                            ERR_BMP280_TIMEOUT;
                    }
                    else
                    {
                        error_flags |=
                            ERR_BMP280_COMM;
                    }

                    error_count++;

                    FSM_ShowErrorOutput();

                    UART_WriteString(
                        "[FSM] BMP280 init failed -> ERROR\r\n");

                    current_state = FSM_ERROR;
                    break;
                }
            }


            /*
             * Read both sensors.
             */
            dht_result =
                DHT11_Read(
                    &current_data.temperature,
                    &current_data.humidity);

            bmp_result =
                BMP280_ReadPressure(
                    &current_data.pressure);

            if ((dht_result == DHT11_OK) &&
                (bmp_result == BMP280_OK))
            {
                UART_WriteString(
                    "[FSM] READ_SENSOR -> VALIDATE\r\n");

                current_state = FSM_VALIDATE;
            }
            else
            {
                /*
                 * DHT11 error mapping.
                 */
                if (dht_result == DHT11_TIMEOUT)
                {
                    error_flags |=
                        ERR_DHT11_TIMEOUT;
                }
                else if (dht_result ==
                         DHT11_CHECKSUM_ERROR)
                {
                    error_flags |=
                        ERR_DHT11_CHECKSUM;
                }


                /*
                 * BMP280 error mapping.
                 *
                 * If BMP communication fails,
                 * force BMP initialization again
                 * on the next retry.
                 */
                if (bmp_result == BMP280_TIMEOUT)
                {
                    error_flags |=
                        ERR_BMP280_TIMEOUT;

                    bmp_initialized = false;
                }
                else if (bmp_result != BMP280_OK)
                {
                    error_flags |=
                        ERR_BMP280_COMM;

                    bmp_initialized = false;
                }

                error_count++;

                FSM_ShowErrorOutput();

                UART_WriteString(
                    "[FSM] READ_SENSOR -> ERROR\r\n");

                current_state = FSM_ERROR;
            }

            break;
        }


        /* ==================================================
         * VALIDATE
         * ================================================== */
        case FSM_VALIDATE:
        {
            current_data.valid =
                SensorData_Validate(
                    &current_data);

            if (current_data.valid)
            {
                UART_WriteString(
                    "[FSM] VALIDATE -> EVALUATE\r\n");

                current_state = FSM_EVALUATE;
            }
            else
            {
                error_flags |=
                    ERR_DATA_INVALID;

                error_count++;

                FSM_ShowErrorOutput();

                UART_WriteString(
                    "[FSM] VALIDATE -> ERROR\r\n");

                current_state = FSM_ERROR;
            }

            break;
        }


        /* ==================================================
         * EVALUATE
         * ================================================== */
        case FSM_EVALUATE:
        {
            SystemStatus instant_status =
                EvaluateState(&current_data);

            /*
            * If the current valid sample agrees with the
            * already-confirmed status, there is no pending
            * transition.
            */
            if (instant_status == current_status)
            {
                candidate_status = current_status;
                candidate_count = 0U;
            }
            else
            {
                /*
                * Same candidate appears again.
                */
                if (instant_status == candidate_status)
                {
                    candidate_count++;
                }
                else
                {
                    /*
                    * A different candidate appeared.
                    * Start counting again from sample 1.
                    */
                    candidate_status = instant_status;
                    candidate_count = 1U;
                }

                /*
                * Commit transition only after
                * 3 consecutive valid samples.
                */
                if (candidate_count >= STATUS_CONFIRM_COUNT)
                {
                    current_status = candidate_status;

                    candidate_status = current_status;
                    candidate_count = 0U;
                }
            }

            UART_WriteString(
                "[FSM] EVALUATE -> UPDATE_OUTPUT\r\n");

            current_state = FSM_UPDATE_OUTPUT;

            break;
        }


        /* ==================================================
         * UPDATE OUTPUT
         * ================================================== */
        case FSM_UPDATE_OUTPUT:
        {
            Output_SetStatus(
                current_status);

            Output_UpdateLCD(
                &current_data,
                current_status,
                ERR_NONE);

            /*
             * A complete successful cycle means
             * the system has recovered.
             */
            error_count = 0U;
            error_flags = ERR_NONE;

            UART_WriteString(
                "[FSM] UPDATE_OUTPUT -> IDLE\r\n\r\n");

            current_state = FSM_IDLE;

            break;
        }


        /* ==================================================
         * ERROR
         * ================================================== */
        case FSM_ERROR:
        {
            /*
             * Too many consecutive failed cycles:
             * enter persistent SENSOR_FAULT.
             */
            if (error_count >=
                FSM_ERROR_THRESHOLD)
            {
                UART_WriteString(
                    "[FSM] ERROR -> SENSOR_FAULT\r\n");

                current_state =
                    FSM_SENSOR_FAULT;

                break;
            }


            /*
             * Transient error:
             * retry on the next sampling period.
             */
            if ((HAL_GetTick() - last_sample_tick)
                    >= FSM_SAMPLE_PERIOD_MS)
            {
                last_sample_tick =
                    HAL_GetTick();

                UART_WriteString(
                    "[FSM] ERROR -> READ_SENSOR (retry)\r\n");

                current_state =
                    FSM_READ_SENSOR;
            }

            break;
        }


        /* ==================================================
         * SENSOR FAULT
         * ================================================== */
        case FSM_SENSOR_FAULT:
        {
            /*
             * SENSOR_FAULT is not a dead state.
             *
             * Continue retrying so that the system
             * can automatically recover.
             */
            if ((HAL_GetTick() - last_sample_tick)
                    >= FSM_SAMPLE_PERIOD_MS)
            {
                last_sample_tick =
                    HAL_GetTick();

                UART_WriteString(
                    "[FSM] SENSOR_FAULT -> READ_SENSOR (recovery retry)\r\n");

                current_state =
                    FSM_READ_SENSOR;
            }

            break;
        }


        /* ==================================================
         * FALLBACK
         * ================================================== */
        default:
        {
            current_state = FSM_IDLE;

            break;
        }
    }
}