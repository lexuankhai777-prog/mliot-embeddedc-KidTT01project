#include "lcd.h"

// Hàm phụ trợ tạo delay tương đối
static void LCD_Delay(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 8000; i++);
}

// Hàm đẩy 1 byte lệnh xuống LCD qua I2C
static void LCD_Write_Cmd(uint8_t cmd) {
    uint8_t data_u = cmd & 0xF0;
    uint8_t data_l = (cmd << 4) & 0xF0;
    uint8_t data_t[4];
    
    data_t[0] = data_u | 0x0C;  // EN=1, RS=0
    data_t[1] = data_u | 0x08;  // EN=0, RS=0
    data_t[2] = data_l | 0x0C;  // EN=1, RS=0
    data_t[3] = data_l | 0x08;  // EN=0, RS=0
    
(void)I2C_Write(SLAVE_ADDRESS_LCD, data_t, 4);}

// Hàm đẩy 1 ký tự hiển thị xuống LCD qua I2C
static void LCD_Write_Data(uint8_t data) {
    uint8_t data_u = data & 0xF0;
    uint8_t data_l = (data << 4) & 0xF0;
    uint8_t data_t[4];
    
    data_t[0] = data_u | 0x0D;  // EN=1, RS=1
    data_t[1] = data_u | 0x09;  // EN=0, RS=1
    data_t[2] = data_l | 0x0D;  // EN=1, RS=1
    data_t[3] = data_l | 0x09;  // EN=0, RS=1
    
 (void)I2C_Write(SLAVE_ADDRESS_LCD, data_t, 4);
}

void LCD_Init(void) {
    LCD_Delay(50); 
    LCD_Write_Cmd(0x33); 
    LCD_Write_Cmd(0x32);
    LCD_Write_Cmd(0x28); // Chế độ 4-bit, 2 dòng
    LCD_Write_Cmd(0x0C); // Bật màn hình, tắt con trỏ
    LCD_Write_Cmd(0x06); // Tự động tăng địa chỉ
    LCD_Clear();
}

void LCD_Clear(void) {
    LCD_Write_Cmd(0x01); // Xóa màn hình
    LCD_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t column) {
    uint8_t address = (row == 0) ? (0x80 + column) : (0xC0 + column);
    LCD_Write_Cmd(address);
}

void LCD_Print(const char *text) {
    while (*text) {
        LCD_Write_Data((uint8_t)(*text));
        text++;
    }
}