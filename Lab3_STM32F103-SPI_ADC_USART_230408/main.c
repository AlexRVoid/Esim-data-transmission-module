/* Дата последней редакции: 09 марта 2023 года
Заказчик:
Плата: Blue Pill
Микроконтроллер: STM32F103C6T6
Язык программирования: Си + работа с регистрами + CMSIS
Автор: Трубин И.В.

Передаёт в USART1 (9600 бод) время, которое отсчитывается по таймеру.
Мигает светодиодом PC13.

============ Входы цифровые ============

============ Выходы цифровые ============


=============== Служебные ===============
USART1: PA9 = TX1, PA10 = RX1
*/                       

#include "stm32f10x.h"
#include "main_functions.c" 
#include "main_init.c"

// ================== Описание глобальных переменных =============
volatile uint16_t Timer = 0;
uint8_t mass[6], i;
volatile uint16_t result1, result2, tmp;


// ============== Подпрограммы обработки прерывания ==============
// ============ USART1 в данном проекте не используется ==========
// USART1->SR, бит RXNE сбрасывается автоматически при чтении USART1->DR,
// записывать в него ноль нужно только при мультибуферной коммуниуации.
void USART1_IRQHandler() {
static u8 ch;
  ch = USART1->DR; USART1->DR = ch + 1;
}  

// =================== TIM2 0.5 сек. ===================
void TIM2_IRQHandler() {
  if ((TIM2->SR & TIM_SR_UIF) != 0) { // Прерывание по переполнению
    TIM2->SR &= ~ TIM_SR_UIF; 
    Timer++; // Наш счётчик времени
  //  GPIOC->ODR ^= GPIO_ODR_ODR13; // Мигаем светодиодом PC8
  }
}  

uint16_t Spi_Write_16b(uint16_t data)
{
    //ждём пока опустошится Tx буфер
	while(!(SPI1->SR & SPI_SR_TXE));
	//активируем Chip Select (Open Drain -> reset)
	GPIOA->BSRR = GPIO_BSRR_BR4;
    //отправляем данные     
	SPI1->DR = data;  

    //ждём пока придёт ответ
	while(!(SPI1->SR & SPI_SR_RXNE));
    //считываем полученные данные
	data = SPI1->DR;  
	//деактивируем Chip Select (Open Drain -> set)
	GPIOA->BSRR = GPIO_BSRR_BS4; 
    //возвращаем то, что прочитали
    return data;  
}

// =================== Очистка матрицы на индикаторе MAX7219 ===================
void ClearMAX7219(void) {
tmp=Spi_Write_16b(0x0800); tmp=Spi_Write_16b(0x0700); tmp=Spi_Write_16b(0x0600);
tmp=Spi_Write_16b(0x0500); tmp=Spi_Write_16b(0x0400); tmp=Spi_Write_16b(0x0300);
tmp=Spi_Write_16b(0x0200); tmp=Spi_Write_16b(0x0100);
}



// =========================== MAIN =========================
int main(void) {
main_init(); // Инициализация МК

// ========================= Инициализация MAX7219 =========================
GPIOA->BSRR = GPIO_BSRR_BS4; //деактивируем Chip Select (Open Drain -> set)
tmp=Spi_Write_16b(0x0F00); tmp=Spi_Write_16b(0x0C01); tmp=Spi_Write_16b(0x0A0F);
tmp=Spi_Write_16b(0x0B07); tmp=Spi_Write_16b(0x0900); 
// Очистка матрицы
ClearMAX7219();

// ========================= ГЛАВНЫЙ ЦИКЛ =========================

 while (1) {
		GPIOC->ODR ^= GPIO_ODR_ODR13; // Мигаем светодиодом PC8
		delay_ms(500);
		I2C1->CR1 |= I2C_CR1_START;
		while (!(I2C1->SR1 & I2C_SR1_SB));
		while (!(I2C1->SR1 & I2C_SR1_TXE));
		I2C1->DR = 0b10100001;
		while (!(I2C1->SR1 & (I2C_SR1_BTF | I2C_SR1_AF)));
		I2C1->CR1 |= I2C_CR1_ACK;
		I2C1->CR1 |= I2C_CR1_STOP;
		while (I2C1->CR1 & I2C_CR1_STOP);
	}
} 

// КОНЕЦ