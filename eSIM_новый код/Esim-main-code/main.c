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
volatile uint8_t TCPConnectCommand[50] = {'A', 'T', '+' ,'C', 'I', 'P', 'O', 'P', 'E', 'N', '=', '1', ',', '\"', 'T', 'C', 'P', '\"', ',', '\"'};
volatile uint8_t TCPCommandReference[3] = {'T', 'C', 'P'};
volatile uint8_t sendTCPMessageCommand[13] = {'A', 'T', '+', 'C', 'I', 'P', 'S', 'E', 'N', 'D', '=', '1', 0x0D};






// ============== Подпрограммы обработки прерывания ==============
// ============ USART1 в данном проекте не используется ==========
// USART1->SR, бит RXNE сбрасывается автоматически при чтении USART1->DR,
// записывать в него ноль нужно только при мультибуферной коммуниуации.



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
	
	while(x < responseIndex){
		
		while(responseBuffer[x] == 62){
			
			
			Send_command_from_uart_to_simcom(message, messageLength);
			delay_ms(5000);
			USART1->DR = 0x1A;
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';
			return 1;
			
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
	
	for (i = 0; i < commandSize; i++){
		
		if(command[i] == '+'){
			x = 0;
			
			while (command[i] == newSMSReferenceResponse[x]){
				i++;
				x++;
				
				if (x == 6) {
					
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
		//максимальная длинна сообщения 170 символов, только латиница
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
	i = 0;
	uint8_t TCPResponseIndex = 0;
	while((command[i] == TCPCommandReference[i]) && (i < 3)){
		TCPResponseIndex++;
		i++;
		if (i == 3){
			commandNum = 2;
		}
	}
	if (commandNum == 2){
		
		volatile uint8_t ip[15];
		volatile uint8_t port[5];
		//TCP,ip:217.71.129.139,p:4742,m:...
		//максимальная длинна сообщения 150 символов, только латиница
		uint8_t x = 0;
		while(command[x] != 'i' && x < commandSize){
			x++;
		}
		x++;
		x++;
		x++;
		for(i = 0; command[x] != ','; i++, x++){
			ip[i] = command[x];
		}
		while(command[x] != 'p' && x < commandSize){
			x++;
		}
		x++;
		x++;
		for(i = 0; command[x] != ','; i++, x++){
			port[i] = command[x];
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
			Send_TCP(ip,port, SMSLength);
			return 1;
		}
		
	}
	return 0;
}

void TCP/IP_Connect(uint8_t ip[], uint8_t port[]){
	a = 0;
	uint8_t x = 20;
	responseIndex = 0;
	for(i = 0; ip != '\n' && i < 15; i++, x++){
		TCPConnectCommand[x] = ip[i];
	}
	
	TCPConnectCommand[x] = '\"';
	x++;
	TCPConnectCommand[x] = ',';
	for(i = 0; port != '\n' && port < 5; i++, x++){
		TCPConnectCommand[x] = port[i];
	}
	
	TCPConnectCommand[x] = 0x0D;
	responseIndex = 0;
	Send_command_from_uart_to_simcom(TCPConnectCommand, x);
}

uint8_t Send_TCP(uint8_t ip, uint8_t port, volatile uint8_t SMSLength){
	TCP/IP_Connect(ip, port);
	Send_command_from_uart_to_simcom(sendTCPMessageCommand, 13);
	a = 0;
	responseIndex = 0;
	sendTCPCommandResponseDelay = 10;
	while((a == 0 && sendDelay != 0) || sendTCPCommandResponseDelay != 0){}
	if (a == 0){
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 'E';return 0;
	}
	x = 0;
	//USART2->DR = '1';
	while(x < responseIndex){
		while(responseBuffer[x] == 62){
			
			Send_command_from_uart_to_simcom(message, messageLength);
			delay_ms(5000);
			USART1->DR = 0x1A;
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';
			return 1;
			
		}
		x++;
	}
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 'E';
	return 0;
	
	
	
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
	
	responseBuffer[responseIndex++] = ch1; // Добавляем символ в массив команды
	
	sendDelay = 18;
	a = 1;
	
}


void USART2_IRQHandler() {
	static u8 ch2;
	ch2 = USART2->DR; 
	
	commandBuffer[commandIndex++] = ch2; // Добавляем символ в массив команды
	
	sendDelay = 4;
	b = 1;
	
}  




// =================== TIM2 0.5 сек. ===================
void TIM2_IRQHandler() {
  if ((TIM2->SR & TIM_SR_UIF) != 0) { // Прерывание по переполнению
    TIM2->SR &= ~ TIM_SR_UIF; 
    Timer++; // Наш счётчик времени
    GPIOC->ODR ^= GPIO_ODR_ODR13; // Мигаем светодиодом PC8
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
main_init(); // Инициализация МК

// ========================= ГЛАВНЫЙ ЦИКЛ =========================

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

// КОНЕЦ