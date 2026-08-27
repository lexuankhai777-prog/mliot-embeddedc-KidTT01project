#include "bmp280.h"
#include "i2c_lowlevel.h"

#include <stddef.h>

/* BMP280 registers */
#define BMP280_REG_CALIB_START   0x88U
#define BMP280_REG_CHIP_ID       0xD0U
#define BMP280_REG_CTRL_MEAS     0xF4U
#define BMP280_REG_CONFIG        0xF5U
#define BMP280_REG_PRESS_MSB     0xF7U

#define BMP280_CHIP_ID_VALUE     0x58U


static uint8_t bmp280_address = BMP280_ADDRESS_76;

/* Temperature calibration coefficients */
static uint16_t dig_T1;
static int16_t  dig_T2;
static int16_t  dig_T3;

/* Pressure calibration coefficients */
static uint16_t dig_P1;
static int16_t  dig_P2;
static int16_t  dig_P3;
static int16_t  dig_P4;
static int16_t  dig_P5;
static int16_t  dig_P6;
static int16_t  dig_P7;
static int16_t  dig_P8;
static int16_t  dig_P9;

static int32_t t_fine;


/* ---------------------------------------------------------- */

static uint16_t BMP280_U16_LE(const uint8_t *data)
{
    return (uint16_t)data[0] |
           ((uint16_t)data[1] << 8);
}

static int16_t BMP280_S16_LE(const uint8_t *data)
{
    return (int16_t)BMP280_U16_LE(data);
}


/* ---------------------------------------------------------- */

static BMP280_Result BMP280_MapI2CResult(I2C_Result result)
{
    switch (result)
    {
        case I2C_RESULT_OK:
            return BMP280_OK;

        case I2C_RESULT_TIMEOUT:
            return BMP280_TIMEOUT;

        case I2C_RESULT_NACK:
        case I2C_RESULT_ERROR:
        default:
            return BMP280_COMM_ERROR;
    }
}


/* ---------------------------------------------------------- */
/* Detect 0x76 / 0x77 and verify chip ID = 0x58               */
/* ---------------------------------------------------------- */

static BMP280_Result BMP280_Detect(void)
{
    uint8_t chip_id = 0U;

    I2C_Result result;

    /* Try 0x76 first */
    result = I2C_MemRead(BMP280_ADDRESS_76,
                         BMP280_REG_CHIP_ID,
                         &chip_id,
                         1U);

    if ((result == I2C_RESULT_OK) &&
        (chip_id == BMP280_CHIP_ID_VALUE))
    {
        bmp280_address = BMP280_ADDRESS_76;
        return BMP280_OK;
    }

    /* Try 0x77 */
    chip_id = 0U;

    result = I2C_MemRead(BMP280_ADDRESS_77,
                         BMP280_REG_CHIP_ID,
                         &chip_id,
                         1U);

    if ((result == I2C_RESULT_OK) &&
        (chip_id == BMP280_CHIP_ID_VALUE))
    {
        bmp280_address = BMP280_ADDRESS_77;
        return BMP280_OK;
    }

    if (result == I2C_RESULT_TIMEOUT)
    {
        return BMP280_TIMEOUT;
    }

    return BMP280_INIT_ERROR;
}


/* ---------------------------------------------------------- */
/* Read factory calibration coefficients                      */
/* ---------------------------------------------------------- */

static BMP280_Result BMP280_ReadCalibration(void)
{
    uint8_t calibration[24];

    I2C_Result result =
        I2C_MemRead(bmp280_address,
                    BMP280_REG_CALIB_START,
                    calibration,
                    sizeof(calibration));

    if (result != I2C_RESULT_OK)
    {
        return BMP280_MapI2CResult(result);
    }

    dig_T1 = BMP280_U16_LE(&calibration[0]);
    dig_T2 = BMP280_S16_LE(&calibration[2]);
    dig_T3 = BMP280_S16_LE(&calibration[4]);

    dig_P1 = BMP280_U16_LE(&calibration[6]);
    dig_P2 = BMP280_S16_LE(&calibration[8]);
    dig_P3 = BMP280_S16_LE(&calibration[10]);
    dig_P4 = BMP280_S16_LE(&calibration[12]);
    dig_P5 = BMP280_S16_LE(&calibration[14]);
    dig_P6 = BMP280_S16_LE(&calibration[16]);
    dig_P7 = BMP280_S16_LE(&calibration[18]);
    dig_P8 = BMP280_S16_LE(&calibration[20]);
    dig_P9 = BMP280_S16_LE(&calibration[22]);

    /*
     * dig_P1 == 0 would later cause division by zero.
     */
    if (dig_P1 == 0U)
    {
        return BMP280_INIT_ERROR;
    }

    return BMP280_OK;
}


/* ---------------------------------------------------------- */

BMP280_Result BMP280_Init(void)
{
    BMP280_Result result = BMP280_Detect();

    if (result != BMP280_OK)
    {
        return result;
    }

    result = BMP280_ReadCalibration();

    if (result != BMP280_OK)
    {
        return result;
    }

    /*
     * ctrl_meas = 0x27
     *
     * osrs_t = x1
     * osrs_p = x1
     * mode   = normal
     */
    uint8_t ctrl_meas = 0x27U;

    I2C_Result i2c_result =
        I2C_MemWrite(bmp280_address,
                     BMP280_REG_CTRL_MEAS,
                     &ctrl_meas,
                     1U);

    if (i2c_result != I2C_RESULT_OK)
    {
        return BMP280_MapI2CResult(i2c_result);
    }

    /*
     * config = 0xA0
     *
     * standby = 1000 ms
     * filter = off
     */
    uint8_t config = 0xA0U;

    i2c_result =
        I2C_MemWrite(bmp280_address,
                     BMP280_REG_CONFIG,
                     &config,
                     1U);

    if (i2c_result != I2C_RESULT_OK)
    {
        return BMP280_MapI2CResult(i2c_result);
    }

    HAL_Delay(10);

    return BMP280_OK;
}


/* ---------------------------------------------------------- */
/* Datasheet temperature compensation                         */
/* ---------------------------------------------------------- */

static int32_t BMP280_CompensateTemperature(int32_t adc_T)
{
    int32_t var1;
    int32_t var2;

    var1 =
        ((((adc_T >> 3) -
           ((int32_t)dig_T1 << 1))) *
          ((int32_t)dig_T2)) >> 11;

    var2 =
        (((((adc_T >> 4) -
            ((int32_t)dig_T1)) *
           ((adc_T >> 4) -
            ((int32_t)dig_T1))) >> 12) *
         ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;

    return (t_fine * 5 + 128) >> 8;
}


/* ---------------------------------------------------------- */
/* Datasheet pressure compensation                            */
/* Output: pressure in Q24.8 Pa                               */
/* ---------------------------------------------------------- */

static uint32_t BMP280_CompensatePressure(int32_t adc_P)
{
    int64_t var1;
    int64_t var2;
    int64_t pressure;

    var1 = ((int64_t)t_fine) - 128000;

    var2 =
        var1 * var1 * (int64_t)dig_P6;

    var2 +=
        ((var1 * (int64_t)dig_P5) << 17);

    var2 +=
        (((int64_t)dig_P4) << 35);

    var1 =
        ((var1 * var1 * (int64_t)dig_P3) >> 8) +
        ((var1 * (int64_t)dig_P2) << 12);

    var1 =
        (((((int64_t)1) << 47) + var1) *
         ((int64_t)dig_P1)) >> 33;

    if (var1 == 0)
    {
        return 0U;
    }

    pressure =
        1048576 - adc_P;

    pressure =
        (((pressure << 31) - var2) * 3125) /
        var1;

    var1 =
        (((int64_t)dig_P9) *
         (pressure >> 13) *
         (pressure >> 13)) >> 25;

    var2 =
        (((int64_t)dig_P8) *
         pressure) >> 19;

    pressure =
        ((pressure + var1 + var2) >> 8) +
        (((int64_t)dig_P7) << 4);

    return (uint32_t)pressure;
}


/* ---------------------------------------------------------- */

BMP280_Result BMP280_ReadPressure(float *pressure_hpa)
{
    if (pressure_hpa == NULL)
    {
        return BMP280_COMM_ERROR;
    }

    uint8_t raw[6];

    I2C_Result result =
        I2C_MemRead(bmp280_address,
                    BMP280_REG_PRESS_MSB,
                    raw,
                    sizeof(raw));

    if (result != I2C_RESULT_OK)
    {
        return BMP280_MapI2CResult(result);
    }

    /*
     * Registers F7-F9 = pressure
     * Registers FA-FC = temperature
     */

    int32_t adc_P =
        ((int32_t)raw[0] << 12) |
        ((int32_t)raw[1] << 4) |
        ((int32_t)raw[2] >> 4);

    int32_t adc_T =
        ((int32_t)raw[3] << 12) |
        ((int32_t)raw[4] << 4) |
        ((int32_t)raw[5] >> 4);

    /*
     * Pressure compensation requires t_fine,
     * so temperature compensation must run first.
     */
    (void)BMP280_CompensateTemperature(adc_T);

    uint32_t pressure_q24_8 =
        BMP280_CompensatePressure(adc_P);

    if (pressure_q24_8 == 0U)
    {
        return BMP280_COMM_ERROR;
    }

    float pressure_pa =
        ((float)pressure_q24_8) / 256.0f;

    *pressure_hpa =
        pressure_pa / 100.0f;

    return BMP280_OK;
}


/* ---------------------------------------------------------- */

uint8_t BMP280_GetAddress(void)
{
    return bmp280_address;
}