/* Дата последней редакции: 04 ноября 2023 года
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
#define BUFFER_SIZE 255
volatile uint16_t Timer = 0;
volatile uint8_t mass[6], commandIndex = 0, i = 0;
volatile uint8_t x = 0, y = 0, a = 0;
volatile uint8_t ifrx = 0;
volatile uint8_t commandBuffer[BUFFER_SIZE], lineAccepted = 0;



// ============== Подпрограммы обработки прерывания ==============
// ============ USART1 в данном проекте не используется ==========
// USART1->SR, бит RXNE сбрасывается автоматически при чтении USART1->DR,
// записывать в него ноль нужно только при мультибуферной коммуниуации.

void Send_command(){
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	for (i = 0; i <= commandIndex; i++){
		USART1->DR = commandBuffer[i];
	}
	commandIndex = 0;
	a = 0;
	
}

void USART1_IRQHandler() {
	static u8 ch;
	ch = USART1->DR; USART1->DR = ch;
	commandBuffer[commandIndex] = ch; // Добавляем символ в массив команды
	//USART1->DR = commandIndex;
	commandIndex++;
	if (ch  == '!'){
		a = 1;
		USART1->DR = 'i';
		//Send_command(commandIndex);
		//a = 1;
	}
	
	
	//USART2->DR = USART1->DR;
	//USART2->DR = ch;
	
}  



void USART2_IRQHandler() {
	//static u8 ch;
	//ch = USART2->DR; 
	//USART2->DR = ch + 1;
	//USART1->DR = USART2->DR;
	//USART1->DR = '!';
}  
// =================== TIM2 0.5 сек. ===================
void TIM2_IRQHandler() {
  if ((TIM2->SR & TIM_SR_UIF) != 0) { // Прерывание по переполнению
    TIM2->SR &= ~ TIM_SR_UIF; 
    Timer++; // Наш счётчик времени
    GPIOC->ODR ^= GPIO_ODR_ODR13; // Мигаем светодиодом PC8
  }
}  

// =========================== MAIN =========================
int main(void) {
main_init(); // Инициализация МК

// ========================= ГЛАВНЫЙ ЦИКЛ =========================

while (1) {
	
	if (a == 1){
		Send_command();
	}
	
	// while (((USART1->SR & USART_SR_RXNE) != 0)){
		// if (USART1->DR != '!') {
			// // Проверяем, не превышен ли размер буфера
			// commandBuffer[commandIndex++] = USART1->DR; // Добавляем символ в массив команды
			
		// }
		// else {
			// lineAccepted = 1;
			// USART2->DR = '\r';
			// USART2->DR = lineAccepted;
		// }
	// }
	// if (lineAccepted && (commandIndex != 0)){
		// USART2->DR = commandIndex;
		// for (i = 0; i < commandIndex; i++) {
            // while ((USART2->SR & USART_SR_TXE) == 0) {}; // Проверяем, готов ли USART2 к передаче данных
            // USART2->DR = commandBuffer[i]; // Отправляем символ через USART2
        // }
		// lineAccepted = 0;
		// commandIndex = 0;
	// }

    // // Проверяем, не получен ли символ '!', который является признаком конца команды
    // if (receivedChar != '!') {
        // // Проверяем, не превышен ли размер буфера
        // if (commandIndex < BUFFER_SIZE - 1) {
            // commandBuffer[commandIndex++] = receivedChar; // Добавляем символ в массив команды
        // }
    // } else if(commandIndex != 0) {
        // // При получении символа '!', отправляем содержимое массива через USART2
        // for (i = 0; i < commandIndex; i++) {
            // while ((USART2->SR & USART_SR_TXE) == 0) {}; // Проверяем, готов ли USART2 к передаче данных
            // USART2->DR = commandBuffer[i]; // Отправляем символ через USART2
        // }
        // // Добавляем символ '!' в конец отправленной команды
        // //while (((USART2->SR & USART_SR_TXE) == 0) {};
        // //USART2->DR = '!';
        // // Очищаем буфер команды для следующей команды
        // commandIndex = 0;
    // }
	
	//x = Rx1();
	
		// i=0;
		// while (((USART1->SR & USART_SR_RXNE) != 0)){
			// atcomm[i] = USART1->DR;
			// i++;
		// }
		// atcomm[i] = '!';
		
		// i=0;
		// while(((USART1->SR & USART_SR_TXE) != 0) ){
			// USART1->DR = atcomm[i];
			// i++;
		// }
		//Tx2Str(atcomm);
		//USART2->DR = x; // Передаем данные с ASCII кодом +1
		// Передатчик готов?
		//x = USART1->DR; // Бит RXNE сбрасывается автоматически при чтении USART1->DR
		
	 // Есть ли данные в буфере по приему
	
	
	// if ((usart2->sr & usart_sr_txe) != 0 && ifrx) {
		// i=0;
		// while(atcomm[i] != '!'){
			// usart2->dr = atcomm[i];
			// i++;
		// }
		
		// //usart2->dr = x; // передаем данные с ascii кодом +1
		// ifrx = 0;
	// }; // передатчик готов?
	
	//delay_ms(500);
/*
//x = Rx1();
	while ((USART2->SR & USART_SR_RXNE) == 0) {}; // Есть ли данные в буфере по приему
	y = USART2->DR; // Бит RXNE сбрасывается автоматически при чтении USART1->DR
	
	//Tx1(x);
	while ((USART1->SR & USART_SR_TXE) == 0) {}; // Передатчик готов?
	USART1->DR = y; // Передаем данные с ASCII кодом +1
	*/


	

	//Tx1(x+1);
	
	//USART2->DR = '\r';
	//Tx2('!'); // Передаем ASCII код символа !

	
	//delay_ms(500);
	//Tx1('\r'); // Передаем данные для переноса строки
	/*
	// Передаем в функию Uint16ToStr значение переменной Timer
	Uint16ToStr(Timer, &mass[0]); 
	// Конец строки *необходимо для корректной работы функции Uint16ToStr
	mass[5] = 0;
	
	// Передаем данные из функции Uint16ToStr по RS-232
	Tx1Str( &mass[0] ); 
	
	Tx1('\r'); // Передаем данные для переноса строки
	delay_ms(500);
	*/
}
}

// КОНЕЦ