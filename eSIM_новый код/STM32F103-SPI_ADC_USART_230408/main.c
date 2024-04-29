/* ���� ��������� ��������: 04 ������ 2023 ����
��������:
�����: Blue Pill
���������������: STM32F103C6T6
���� ����������������: �� + ������ � ���������� + CMSIS
�����: ������ �.�.

������� � USART1 (9600 ���) �����, ������� ������������� �� �������.
������ ����������� PC13.

============ ����� �������� ============

============ ������ �������� ============


=============== ��������� ===============
USART1: PA9 = TX1, PA10 = RX1
*/                       

#include "stm32f10x.h"
#include "main_functions.c" 
#include "main_init.c"

// ================== �������� ���������� ���������� =============
#define BUFFER_SIZE 255
volatile uint16_t Timer = 0;
volatile uint8_t mass[6], commandIndex = 0, i = 0;
volatile uint8_t x = 0, y = 0, a = 0;
volatile uint8_t ifrx = 0;
volatile uint8_t commandBuffer[BUFFER_SIZE], lineAccepted = 0;



// ============== ������������ ��������� ���������� ==============
// ============ USART1 � ������ ������� �� ������������ ==========
// USART1->SR, ��� RXNE ������������ ������������� ��� ������ USART1->DR,
// ���������� � ���� ���� ����� ������ ��� �������������� ������������.

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
	commandBuffer[commandIndex] = ch; // ��������� ������ � ������ �������
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
// =================== TIM2 0.5 ���. ===================
void TIM2_IRQHandler() {
  if ((TIM2->SR & TIM_SR_UIF) != 0) { // ���������� �� ������������
    TIM2->SR &= ~ TIM_SR_UIF; 
    Timer++; // ��� ������� �������
    GPIOC->ODR ^= GPIO_ODR_ODR13; // ������ ����������� PC8
  }
}  

// =========================== MAIN =========================
int main(void) {
main_init(); // ������������� ��

// ========================= ������� ���� =========================

while (1) {
	
	if (a == 1){
		Send_command();
	}
	
	// while (((USART1->SR & USART_SR_RXNE) != 0)){
		// if (USART1->DR != '!') {
			// // ���������, �� �������� �� ������ ������
			// commandBuffer[commandIndex++] = USART1->DR; // ��������� ������ � ������ �������
			
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
            // while ((USART2->SR & USART_SR_TXE) == 0) {}; // ���������, ����� �� USART2 � �������� ������
            // USART2->DR = commandBuffer[i]; // ���������� ������ ����� USART2
        // }
		// lineAccepted = 0;
		// commandIndex = 0;
	// }

    // // ���������, �� ������� �� ������ '!', ������� �������� ��������� ����� �������
    // if (receivedChar != '!') {
        // // ���������, �� �������� �� ������ ������
        // if (commandIndex < BUFFER_SIZE - 1) {
            // commandBuffer[commandIndex++] = receivedChar; // ��������� ������ � ������ �������
        // }
    // } else if(commandIndex != 0) {
        // // ��� ��������� ������� '!', ���������� ���������� ������� ����� USART2
        // for (i = 0; i < commandIndex; i++) {
            // while ((USART2->SR & USART_SR_TXE) == 0) {}; // ���������, ����� �� USART2 � �������� ������
            // USART2->DR = commandBuffer[i]; // ���������� ������ ����� USART2
        // }
        // // ��������� ������ '!' � ����� ������������ �������
        // //while (((USART2->SR & USART_SR_TXE) == 0) {};
        // //USART2->DR = '!';
        // // ������� ����� ������� ��� ��������� �������
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
		//USART2->DR = x; // �������� ������ � ASCII ����� +1
		// ���������� �����?
		//x = USART1->DR; // ��� RXNE ������������ ������������� ��� ������ USART1->DR
		
	 // ���� �� ������ � ������ �� ������
	
	
	// if ((usart2->sr & usart_sr_txe) != 0 && ifrx) {
		// i=0;
		// while(atcomm[i] != '!'){
			// usart2->dr = atcomm[i];
			// i++;
		// }
		
		// //usart2->dr = x; // �������� ������ � ascii ����� +1
		// ifrx = 0;
	// }; // ���������� �����?
	
	//delay_ms(500);
/*
//x = Rx1();
	while ((USART2->SR & USART_SR_RXNE) == 0) {}; // ���� �� ������ � ������ �� ������
	y = USART2->DR; // ��� RXNE ������������ ������������� ��� ������ USART1->DR
	
	//Tx1(x);
	while ((USART1->SR & USART_SR_TXE) == 0) {}; // ���������� �����?
	USART1->DR = y; // �������� ������ � ASCII ����� +1
	*/


	

	//Tx1(x+1);
	
	//USART2->DR = '\r';
	//Tx2('!'); // �������� ASCII ��� ������� !

	
	//delay_ms(500);
	//Tx1('\r'); // �������� ������ ��� �������� ������
	/*
	// �������� � ������ Uint16ToStr �������� ���������� Timer
	Uint16ToStr(Timer, &mass[0]); 
	// ����� ������ *���������� ��� ���������� ������ ������� Uint16ToStr
	mass[5] = 0;
	
	// �������� ������ �� ������� Uint16ToStr �� RS-232
	Tx1Str( &mass[0] ); 
	
	Tx1('\r'); // �������� ������ ��� �������� ������
	delay_ms(500);
	*/
}
}

// �����