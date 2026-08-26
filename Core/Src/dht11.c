#include "dht11.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;

static void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

static void DHT11_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

static void DHT11_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

void DHT11_Init(void) {
    HAL_TIM_Base_Start(&htim1);
}

DHT11_Result DHT11_Read(float *temperature, float *humidity) {
    uint8_t data[5] = {0};
    uint16_t timeout;

    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
    HAL_Delay(18); 
    
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
    delay_us(20);
    DHT11_SetPinInput();

    timeout = 0;
    while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) {
        timeout++;
        if(timeout > 100) return DHT11_TIMEOUT;
        delay_us(1);
    }
    
    timeout = 0;
    while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET) {
        timeout++;
        if(timeout > 100) return DHT11_TIMEOUT;
        delay_us(1);
    }
    
    timeout = 0;
    while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) {
        timeout++;
        if(timeout > 100) return DHT11_TIMEOUT;
        delay_us(1);
    }

    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 8; j++) {
            timeout = 0;
            while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_RESET) {
                timeout++;
                if(timeout > 100) return DHT11_TIMEOUT;
                delay_us(1);
            }
            
            delay_us(40);
            
            if(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) {
                data[i] |= (1 << (7 - j));
                timeout = 0;
                while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) {
                    timeout++;
                    if(timeout > 100) return DHT11_TIMEOUT;
                    delay_us(1);
                }
            }
        }
    }

    if((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return DHT11_CHECKSUM_ERROR;
    }

    *humidity = (float)data[0] + ((float)data[1] / 10.0f);
    *temperature = (float)data[2] + ((float)data[3] / 10.0f);

    return DHT11_OK;
}