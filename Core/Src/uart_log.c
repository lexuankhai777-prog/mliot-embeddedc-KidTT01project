#include "uart_log.h"

void UART_Init(void) {
    // 1. Cấp xung nhịp (Clock) cho GPIOA và USART1 (Cả 2 đều nằm trên APB2)
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    // 2. Cấu hình chân PA9 (TX) - Chế độ Alternate Function Push-Pull, tốc độ 50MHz
    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);       
    GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9);      

    // 3. Cấu hình chân PA10 (RX) - Chế độ Floating Input
    GPIOA->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);     
    GPIOA->CRH |= GPIO_CRH_CNF10_0;                        

    // 4. Cấu hình tốc độ Baud = 115200 (Cấu hình 8N1)
    // Công thức tính cho USART1 (trên bus APB2): USARTDIV = fCK / (16 * Baudrate)
    // Giả định PCLK2 (APB2) = 72MHz: USARTDIV = 72,000,000 / (16 * 115200) = 39.0625
    // Phần nguyên = 39 (0x27), Phần thập phân = 0.0625 * 16 = 1 (0x1) -> BRR = 0x271
    USART1->BRR = 0x271; 

    // 5. Kích hoạt USART (UE), Bộ truyền (TE), Bộ nhận (RE)
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void UART_WriteString(const char *str) {
    // Duyệt qua từng ký tự của chuỗi cho đến khi gặp NULL (\0)
    while (*str) {
        // Chờ cờ TXE (Transmit Data Register Empty) lên 1 - báo hiệu bộ đệm đã trống
        while (!(USART1->SR & USART_SR_TXE));
        
        // Ghi ký tự vào thanh ghi Data (DR)
        USART1->DR = (*str & 0xFF);
        str++;
    }
    // Chờ cờ TC (Transmission Complete) để đảm bảo byte cuối cùng đã được đẩy ra ngoài
    while (!(USART1->SR & USART_SR_TC));
}