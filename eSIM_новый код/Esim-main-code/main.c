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
volatile uint8_t i = 0;
volatile uint8_t x = 0, y = 0, a = 0, b = 0, sendDelay = 0;
volatile uint8_t commandBuffer[BUFFER_SIZE_COMMAND], responseBuffer[BUFFER_SIZE_RESPONSE];
volatile uint8_t commandIndex = 0, responseIndex = 0;
volatile uint8_t sendSMSCommand[23] = {'A', 'T', '+', 'C', 'M', 'G', 'S', '=', '\"'};
volatile uint8_t readSMSCommand[14] = {'A', 'T', '+', 'C', 'M', 'G', 'R', '='};
volatile uint8_t message[MAX_SMS_LENGTH], sendSMSCommandResponseDelay = 0, sendSMSErr = 0;
volatile uint8_t SMSReferenceResponse[6] = {'+', 'C', 'M', 'G', 'S', ':'};
volatile uint8_t newSMSReferenceResponse[6] = {'+', 'C', 'M', 'T', 'I', ':'};
volatile uint8_t SMSCommandReference[3] = {'S', 'M', 'S'};
volatile uint8_t SMSMessageFormatcommand[10] = {'A', 'T', '+', 'C', 'M', 'G', 'F', '=', ' ', 0x0D};
volatile uint8_t commandNum = 0;
volatile uint8_t lastSMSIndex = 0;
volatile uint8_t commandProcessing = 0;
volatile uint8_t TCPConnectCommand[50] = {'A', 'T', '+' ,'C', 'I', 'P', 'O', 'P', 'E', 'N', '=', '1', ',', '\"', 'T', 'C', 'P', '\"', ',', '\"'};
volatile uint8_t TCPCommandReference[3] = {'T', 'C', 'P'};
volatile uint8_t sendTCPMessageCommand[13] = {'A', 'T', '+', 'C', 'I', 'P', 'S', 'E', 'N', 'D', '=', '1', 0x0D};
volatile uint8_t sendTCPCommandResponseDelay = 0;
volatile uint8_t startSocketServiceCommand[11] = {'A', 'T', '+', 'N', 'E', 'T', 'O', 'P', 'E', 'N', 0x0D};
volatile uint8_t modeToRetrieveDataCommand[14] = {'A', 'T', '+', 'C', 'I', 'P', 'R', 'X', 'G', 'E', 'T', '=', ' ', 0x0D};
volatile uint8_t addIPHeaderCommand[13] = {'A', 'T', '+', 'C', 'I', 'P', 'H', 'E', 'A', 'D', '=', ' ', 0x0D};
volatile uint8_t showRemoteIPAddressAndPortCommand[13] = {'A', 'T', '+', 'C', 'I', 'P', 'S', 'R', 'I', 'P', '=', ' ',0x0D};
volatile uint8_t callCommand[17] = {'A', 'T', 'D'};
volatile uint8_t callCommandReference[4] = {'C', 'A', 'L', 'L'};
volatile uint8_t SIMCardSwitchReference[9] = {'S', 'I', 'M', 'S', 'w', 'i', 't', 'c', 'h'};
volatile uint8_t SIMCardSwitchCommand[15] = {'A', 'T', '+', 'S', 'W', 'I', 'T', 'C', 'H', 'S', 'I', 'M', '=', ' ',0x0D};







// ============== Подпрограммы обработки прерывания ==============
// ============ USART1 в данном проекте не используется ==========
// USART1->SR, бит RXNE сбрасывается автоматически при чтении USART1->DR,
// записывать в него ноль нужно только при мультибуферной коммуниуации.

void SIMCardSwitch(volatile uint8_t simNum){ //Смена основной сим карты.
	SIMCardSwitchCommand[13] = simNum;//Добавление номера слота сим карты к AT команде 
	Send_command_from_uart_to_simcom(SIMCardSwitchCommand, 15);//Отправка АТ команды в simcom
}

void VoiceCall(volatile uint8_t phoneNum[]){ //Голосовой вызов на указанный номер
	for(i = 0; i < 12; i++){//Добавление номера телефона к АТ к команде 
		callCommand[i+3] = phoneNum[i];
	}
	callCommand[15] = ';';
	callCommand[16] = 0x0D;
	Send_command_from_uart_to_simcom(callCommand, 17);//Отправка АТ команды в simcom
}

void StartSocketService(){ //Инициализация веб сокета для работы с сетью и TCP/IP
	
	modeToRetrieveDataCommand[12] = '0';//Установка автоматического вывода сообщенияпринятого по TCP
	Send_command_from_uart_to_simcom(modeToRetrieveDataCommand, 14);//Отправка АТ команды в simcom
	delay_ms(5000);//ожидание отклика команды 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Вывод результата выполнения команды 
	
	addIPHeaderCommand[11] = '1';//Установка IP хедера формата: "+IPD(data length)", при выводе принятых данных по TCP
	Send_command_from_uart_to_simcom(addIPHeaderCommand, 13);//Отправка АТ команды в simcom
	delay_ms(5000);//ожидание отклика команды 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Вывод результата выполнения команды 
	
	showRemoteIPAddressAndPortCommand[11] = '1';//Установка вывода IP и порта при приёме данных по TCP формата: "RECV FROM:<IP ADDRESS>:<PORT>"
	Send_command_from_uart_to_simcom(showRemoteIPAddressAndPortCommand, 13);//Отправка АТ команды в simcom
	delay_ms(5000);//ожидание отклика команды 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Вывод результата выполнения команды 
	
	Send_command_from_uart_to_simcom(startSocketServiceCommand, 11);//Отправка АТ команды открытия сервиса сокетов в simcom
	delay_ms(5000);//ожидание отклика команды 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Вывод результата выполнения команды 
}

uint8_t Send_SMS(volatile uint8_t phoneNum[], volatile uint8_t messageLength){ //Отправка СМС сообщения на указанный номер
	a = 0;
	responseIndex = 0;
	for(i = 0; i < 12; i++){//Добавление номера телефона к АТ к команде 
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
	}	
	
	i = 0;
	uint8_t TCPResponseIndex = 0;
	while((command[i] == TCPCommandReference[i]) && (i < 3)){
		TCPResponseIndex++;
		i++;
		if (i == 3){
			volatile uint8_t ip[15];
			volatile uint8_t port[5];
			//TCP,ip:217.71.129.139,p:4742,m:...
			//максимальная длинна сообщения 170 символов, только латиница
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
	}
	
	uint8_t callResponseIndex = 0;
	while((command[i] == callCommandReference[i]) && (i < 4)){
		callResponseIndex++;
		i++;
		if (i == 4){
			volatile uint8_t phone[12];
			//CALL,n:+79963438273
			uint8_t x = 0;
			while(command[x] != 'n' && x < commandSize){
				x++;
			}
			x++;
			x++;
			for(i = 0; i <= 12; i++, x++){
				phone[i] = command[x];
			}
			VoiceCall(phone);
			return 1;
		}
	}
	
	uint8_t SIMCardSwitchResponseIndex = 0;
	while((command[i] == SIMCardSwitchReference[i]) && (i < 9)){
		SIMCardSwitchResponseIndex++;
		i++;
		if (i == 9){
			volatile uint8_t simNum;
			//SIMSwitch,x
			//x - номер симкарты (0-1)
			//0 - SIM1, 1 - SIM2
			uint8_t x = 0;
			while(command[x] != ',' && x < commandSize){
				x++;
			}
			x++;
			simNum = command[x];
			SIMCardSwitch(simNum);
			return 1;
		}
	}
	
	return 0;
}

void TCP_IP_Connect(volatile uint8_t ip[], volatile uint8_t port[]){
	//Send_command_from_uart_to_simcom(startSocketServiceCommand, 11);
	Send_command_from_simcom_to_uart(startSocketServiceCommand, 11);
	delay_ms(5000);
	//Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	a = 0;
	uint8_t x = 20;
	responseIndex = 0;
	for(i = 0; ip[i] != '\n' && i < 15; i++, x++){
		TCPConnectCommand[x] = ip[i];
	}
	
	TCPConnectCommand[x] = '\"';
	x++;
	TCPConnectCommand[x] = ',';
	
	for(i = 0; port[i] != '\n' && i < 5; i++, x++){
		TCPConnectCommand[x] = port[i];
	}
	
	TCPConnectCommand[x] = 0x0D;
	responseIndex = 0;
	//Send_command_from_uart_to_simcom(TCPConnectCommand, x);
	Send_command_from_simcom_to_uart(TCPConnectCommand, x);
}

void Send_TCP(volatile uint8_t ip[], volatile uint8_t port[], volatile uint8_t messageLength){
	TCP_IP_Connect(ip, port);
	//Send_command_from_uart_to_simcom(sendTCPMessageCommand, 13);
	Send_command_from_simcom_to_uart(sendTCPMessageCommand, 13);
	a = 0;
	responseIndex = 0;
	sendTCPCommandResponseDelay = 10;
	while((a == 0 && sendDelay != 0) || sendTCPCommandResponseDelay != 0){}
	if (a == 0){
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 'E';return;
	}
	x = 0;
	//USART2->DR = '1';
	while(x < responseIndex){
		while(responseBuffer[x] == 62){
			
			//Send_command_from_uart_to_simcom(message, messageLength);
			Send_command_from_simcom_to_uart(message, messageLength);
			delay_ms(5000);
			USART1->DR = 0x1A;
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';
			return;
			
		}
		x++;
	}
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 'E';
	return;
	
	
	
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
  
  if(sendTCPCommandResponseDelay != 0){
	sendTCPCommandResponseDelay--;
  }
  
  if (commandProcessing == 1 && a == 1 && sendDelay == 0){
	  Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
  }
}  

// =========================== MAIN =========================
int main(void) {
main_init(); // Инициализация МК

delay_ms(5000);
GPIOB->BSRR |= GPIO_BSRR_BS5;
delay_ms(1000);
GPIOB->BRR |= GPIO_BRR_BR5;
delay_ms(90000);
Send_command_from_simcom_to_uart(responseBuffer, responseIndex);

SMSMessageFormatcommand[8] = '1';
Send_command_from_uart_to_simcom(SMSMessageFormatcommand, 10);
delay_ms(5000);
Send_command_from_simcom_to_uart(responseBuffer, responseIndex);

StartSocketService();

uint8_t statusReady[7] = {'R', 'e', 'a', 'd', 'y', 0x0D, 0x1A};
Send_command_from_simcom_to_uart(statusReady, 7);


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