#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>
#include "stm32f1xx.h"
#include "app_types.h" 

// Public API đã chốt
void Output_Init(void);
void Output_SetStatus(SystemStatus status);
void Output_ShowError(uint32_t error_flags);
void Output_UpdateLCD(const SensorData *data, SystemStatus status, uint32_t error_flags);

// Hàm bọc Buzzer (Buzzer Wrapper) mới thêm
void Buzzer_On(void);
void Buzzer_Off(void);

#endif /* OUTPUT_H */