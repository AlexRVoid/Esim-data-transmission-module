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
#define BUFFER_SIZE_COMMAND 500
#define BUFFER_SIZE_RESPONSE 1000
#define MAX_SMS_LENGTH 170

volatile uint16_t Timer = 0;
volatile uint8_t command_number = 0, i = 0;
volatile uint8_t x = 0, y = 0, a = 0, b = 0, sendDelay = 0;
volatile uint8_t commandBuffer[BUFFER_SIZE_COMMAND], responseBuffer[BUFFER_SIZE_RESPONSE];
volatile uint8_t commandIndex = 0, responseIndex = 0;
volatile uint8_t sendSMSCommand[23] = {'A', 'T', '+', 'C', 'M', 'G', 'S', '=', '\"'};
volatile uint8_t readSMSCommand[14] = {'A', 'T', '+', 'C', 'M', 'G', 'R', '='};
volatile uint8_t message[MAX_SMS_LENGTH], sendSMSCommandResponseDelay = 0, sendSMSErr = 0;
volatile uint8_t SMSReferenceResponse[6] = {'+', 'C', 'M', 'G', 'S', ':'};
volatile uint8_t newSMSReferenceResponse[6] = {'+', 'C', 'M', 'T', 'I', ':'};
volatile uint8_t SMSCommandReference[3] = {'S', 'M', 'S'};
volatile uint8_t commandNum = 0;
volatile uint8_t lastSMSIndex = 0;
volatile uint8_t commandProcessing = 0;






// ============== ������������ ��������� ���������� ==============
// ============ USART1 � ������ ������� �� ������������ ==========
// USART1->SR, ��� RXNE ������������ ������������� ��� ������ USART1->DR,
// ���������� � ���� ���� ����� ������ ��� �������������� ������������.



uint8_t Send_SMS(volatile uint8_t phoneNum[], volatile uint8_t messageLength){
	a = 0;
	responseIndex = 0;
	for(i = 0; i < 12; i++){
		sendSMSCommand[i+9] = phoneNum[i];
	}
	sendSMSCommand[21] = '\"';
	sendSMSCommand[22] = 0x0D;
	responseIndex = 0;
	Send_command_from_uart_to_simcom(sendSMSCommand, 23);
	sendSMSCommandResponseDelay = 50;
	while((a == 0 && sendDelay != 0) || sendSMSCommandResponseDelay != 0){}
	if (a == 0){
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 'E';return 0;
	}
	x = 0;
	//USART2->DR = '1';
	while(x < responseIndex){
		//while ((USART2->SR & USART_SR_TXE) == 0) {};
		//USART2->DR = '2';
		while(responseBuffer[x] == 62){
			//while ((USART2->SR & USART_SR_TXE) == 0) {};
			//USART2->DR = '3';
			
			Send_command_from_uart_to_simcom(message, messageLength);
			delay_ms(5000);
			USART1->DR = 0x1A;
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';
			return 1;
			/*
			uint8_t SMSResponseIndex = 0;
			while(responseBuffer[SMSResponseIndex] != '+'){
				SMSResponseIndex++;
			}
			i = 0;
			while((responseBuffer[SMSResponseIndex] == SMSReferenceResponse[i]) && (i < 6)){
				SMSResponseIndex++;
				i++;
			}
			if(i == 6){
				USART2->DR = 'S';
			}
			*/
		}
		x++;
	}
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 'E';
	return 0;
	
}
void SMS_search(volatile uint8_t command[], volatile uint8_t commandSize){
	commandProcessing = 1;
	// while ((USART2->SR & USART_SR_TXE) == 0) {};
	// USART2->DR = commandSize+48;
	for (i = 0; i < commandSize; i++){
		//while ((USART2->SR & USART_SR_TXE) == 0) {};
		//USART2->DR = '2';
		if(command[i] == '+'){
			x = 0;
			//while ((USART2->SR & USART_SR_TXE) == 0) {};
			//USART2->DR = '3';
			while (command[i] == newSMSReferenceResponse[x]){
				i++;
				x++;
				//while ((USART2->SR & USART_SR_TXE) == 0) {};
				//USART2->DR = '4';
				if (x == 6) {
					//while ((USART2->SR & USART_SR_TXE) == 0) {};
					//USART2->DR = '5';
					while(command[i] != ','){
						i++;
					}
					i++;
					for (y = 8; command[i] != 0x0D; i++, y++){
						readSMSCommand[y] = command[i];
					}
					readSMSCommand[y] = 0x0D;
					Send_command_from_uart_to_simcom(readSMSCommand, y+1);	
				}
			}
		}
	}
	commandProcessing = 0;
}

uint8_t Command_check(volatile uint8_t command[], volatile uint8_t commandSize){
	i = 0;
	uint8_t SMSResponseIndex = 0;
	while((command[i] == SMSCommandReference[i]) && (i < 3)){
		SMSResponseIndex++;
		i++;
		if (i == 3){
			commandNum = 1;
		}
	}	
	if (commandNum == 1){
		
		volatile uint8_t phone[12];
		//SMS,n:+79963438273,m:...
		//������������ ������ ��������� 170 ��������, ������ ��������
		uint8_t x = 0;
		while(command[x] != 'n' && x < commandSize){
			x++;
		}
		x++;
		x++;
		for(i = 0; i <= 12; i++, x++){
			phone[i] = command[x];
		}
		while(command[x] != 'm' && x < commandSize){
			x++;
		}
		x++;
		x++;
		volatile uint8_t SMSLength = commandSize - x;
		if (SMSLength <= MAX_SMS_LENGTH){
			for (i = 0; x < commandSize; i++, x++){
				message[i] = command[x];
			}
			Send_SMS(phone, SMSLength);
			return 1;
		}
		
	}
	return 0;
}

void TCP/IP_Connect(volatile uint8_t ip[], volatile uint8_t port[]){
	
}
void Send_command_from_simcom_to_uart(volatile uint8_t command[], volatile uint8_t commandSize){
	for (i = 0; i < commandSize; i++){
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = command[i];
	}
	responseIndex = 0;
	a = 0;
}
void Send_command_from_uart_to_simcom(volatile uint8_t command[], volatile uint8_t commandSize){
	
	for (i = 0; i < commandSize; i++){
		while ((USART1->SR & USART_SR_TXE) == 0) {};
		USART1->DR = command[i];
	}
	commandIndex = 0;
	b = 0;
	
}

void USART1_IRQHandler() {
	
	static u8 ch1;
	ch1 = USART1->DR; 
	//USART1->DR = ch;
	responseBuffer[responseIndex++] = ch1; // ��������� ������ � ������ �������
	//USART1->DR = commandIndex;
	//commandIndex++;
	sendDelay = 18;
	a = 1;
	/*
	if (ch  == '!'){
		a = 1;
	}
	*/
}


void USART2_IRQHandler() {
	static u8 ch2;
	ch2 = USART2->DR; 
	//USART1->DR = ch;
	commandBuffer[commandIndex++] = ch2; // ��������� ������ � ������ �������
	//USART1->DR = commandIndex;
	//commandIndex++;
	sendDelay = 4;
	b = 1;
	/*
	if (ch  == '!'){
		b = 1;
		//USART1->DR = 'i';
		//Send_command(commandIndex);
		//a = 1;
	}
	*/
}  




// =================== TIM2 0.5 ���. ===================
void TIM2_IRQHandler() {
  if ((TIM2->SR & TIM_SR_UIF) != 0) { // ���������� �� ������������
    TIM2->SR &= ~ TIM_SR_UIF; 
    Timer++; // ��� ������� �������
    GPIOC->ODR ^= GPIO_ODR_ODR13; // ������ ����������� PC8
  }
  if (sendDelay != 0){
	  sendDelay--;
  }
  
  if(sendSMSCommandResponseDelay != 0){
	  sendSMSCommandResponseDelay--;
  }
  
  if (commandProcessing == 1 && a == 1 && sendDelay == 0){
	  Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
  }
}  

// =========================== MAIN =========================
int main(void) {
main_init(); // ������������� ��

// ========================= ������� ���� =========================

while (1) {
	
	if (a == 1 && sendDelay == 0){
		SMS_search(responseBuffer, responseIndex);
		Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	}
	if (b == 1 && sendDelay == 0){
		if (Command_check(commandBuffer, commandIndex) == 0){
			Send_command_from_uart_to_simcom(commandBuffer, commandIndex);
		}
	}
	
	
}
}

// �����