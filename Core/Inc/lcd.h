#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "i2c_lowlevel.h"

#define SLAVE_ADDRESS_LCD 0x27 // Địa chỉ 7-bit (chưa dịch trái 1 bit)

// Các Public API đã chốt trong Hợp đồng Kỹ thuật
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t column);
void LCD_Print(const char *text);

#endif /* LCD_H */