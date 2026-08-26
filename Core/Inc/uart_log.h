#ifndef UART_LOG_H
#define UART_LOG_H

#include <stdint.h>
#include "stm32f1xx.h"

// Public API theo Hợp đồng nhóm
void UART_Init(void);
void UART_WriteString(const char *str);

#endif /* UART_LOG_H */