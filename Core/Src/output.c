#include <stdio.h>
#include "output.h"
#include "lcd.h"
// --- IMPLEMENT BUZZER WRAPPER ---
void Buzzer_On(void) {
    // Active-low: Xuất mức LOW (0) ra PB15 để BẬT còi
    GPIOB->BSRR = (1U << (15 + 16)); 
}

void Buzzer_Off(void) {
    // Active-low: Xuất mức HIGH (1) ra PB15 để TẮT còi
    GPIOB->BSRR = (1U << 15); 
}

void Output_Init(void) {
    // 1. Cấp xung nhịp cho GPIOB
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // 2. Cấu hình PB12, PB13, PB14, PB15 làm Output Push-Pull 50MHz
    GPIOB->CRH &= ~(0xFFFF0000); 
    GPIOB->CRH |= 0x33330000;    

    // 3. Khởi tạo trạng thái tắt ban đầu
    // Tắt 3 đèn LED (Active-High -> Cần xuất mức 0)
    GPIOB->BSRR = (1U << (12 + 16)) | (1U << (13 + 16)) | (1U << (14 + 16)); 
    // Tắt Còi bằng hàm Wrapper
    Buzzer_Off();
}

void Output_SetStatus(SystemStatus status) {
    // Tắt tất cả LED trước khi bật trạng thái mới
    GPIOB->BSRR = (1U << (12 + 16)) | (1U << (13 + 16)) | (1U << (14 + 16));
    // Tắt Còi bằng hàm Wrapper
    Buzzer_Off();

    // Bật lại theo trạng thái
    switch (status) {
        case STATUS_NORMAL:
            GPIOB->BSRR = (1U << 12); // Bật Green LED
            break;
        case STATUS_WARNING:
            GPIOB->BSRR = (1U << 13); // Bật Yellow LED
            break;
        case STATUS_ALARM:
            GPIOB->BSRR = (1U << 14);           // Bật Red LED (Kích mức 1)
            GPIOB->BSRR = (1U << (15 + 16));    // BẬT CÒI (Kích mức 0 cho Active-Low)
            break;
    }
}

void Output_ShowError(uint32_t error_flags) {
    if (error_flags != 0) {
        GPIOB->BSRR = (1U << (12 + 16)) | (1U << (13 + 16)); // Tắt Xanh, Vàng
        GPIOB->BSRR = (1U << 14);                           // Bật Đỏ
        Buzzer_Off();                                      // Đảm bảo còi tắt khi báo lỗi
    }
}

void Output_UpdateLCD(const SensorData *data, SystemStatus status, uint32_t error_flags) {
    char line_buf[32]; // Buffer chứa tối đa 32 ký tự + null

    if (error_flags != 0) {
        // Dòng 1: Báo trạng thái lỗi cảm biến
        LCD_SetCursor(0, 0);
        LCD_Print(" SENSOR ERROR!  ");
        
        // Dòng 2: Hiển thị mã lỗi chi tiết
        LCD_SetCursor(1, 0);
        snprintf(line_buf,sizeof(line_buf),"Err Code: %-7lu",error_flags);        
         LCD_Print(line_buf);
    } else {
        // --- XỬ LÝ SỐ THỰC THÀNH SỐ NGUYÊN ĐỂ IN ---
        // 1. Tách Nhiệt độ (Ví dụ: 25.5 -> Nguyên: 25, Thập phân: 5)
        int t_int = (int)data->temperature;
        int t_frac = (int)((data->temperature - t_int) * 10);
        if (t_frac < 0) t_frac = -t_frac; // Khử dấu âm cho phần thập phân

        // 2. Tách Độ ẩm
        int h_int = (int)data->humidity;
        int h_frac = (int)((data->humidity - h_int) * 10);

        // 3. Ép kiểu Áp suất (thường áp suất hiển thị số nguyên là đủ)
        int p_int = (int)data->pressure;
        
        // Dòng 1: Nhiệt độ & Độ ẩm 
        LCD_SetCursor(0, 0);
        snprintf(line_buf,sizeof(line_buf),"T:%d.%dC H:%d.%d%% ",t_int,t_frac,h_int,h_frac);
        LCD_Print(line_buf);

        // Dòng 2: Áp suất & Status
        LCD_SetCursor(1, 0);
        char *status_str;
if (status == STATUS_NORMAL) status_str = "NORM";
else if (status == STATUS_WARNING) status_str = "WARN";
else status_str = "ALRM";
        
        sprintf(line_buf, "P:%dhPa %s  ", p_int, status_str);
        LCD_Print(line_buf);
    }
}