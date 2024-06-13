/*Дата последней редакции: 13 Июня 2024 года
Заказчик: АО "РИМ"
Плата: Blue Pill
Микроконтроллер: STM32F103C6T6
Язык программирования: Си + работа с регистрами + CMSIS
Автор: Втюрин А.Р, Сысоев С.А.

Передаёт и обрабатывает поступающие команды с USART2(115200 бод) на USART1(115200 бод).
Мигает светодиодом PC13.

============ Входы цифровые ============
USART1: PA10 = RX1
USART2: PA3 = RX2
============ Выходы цифровые ============
USART1: PA9 = TX1
USART2: PA2 = TX2
Цифровой выход: PB5 = Включение модуля передачи данных
Цифровой выход: PB6 = Перезагрузка модуля передачи данных

*/                       

#include "stm32f10x.h"
#include "main_functions.c" 
#include "main_init.c"


// =================== TIM2 0.5 сек. ===================
void TIM2_IRQHandler() {
  if ((TIM2->SR & TIM_SR_UIF) != 0) { // Прерывание по переполнению
    TIM2->SR &= ~ TIM_SR_UIF; 
    Timer++; // Наш счётчик времени
    GPIOC->ODR ^= GPIO_ODR_ODR13; // Мигаем светодиодом PC8
  }
  if (sendDelay != 0){
	  sendDelay--;//Задержка для получения ответа от simcom или stm
  }
  
  if(sendSMSCommandResponseDelay != 0){
	  sendSMSCommandResponseDelay--;//Задержка для получения ответа от simcom при отправке СМС сообщения
  }
  
  if(sendTCPCommandResponseDelay != 0){
	sendTCPCommandResponseDelay--;//Задержка для получения ответа от simcom при отправке данных по TCP
  }
  
  if (commandProcessing == 1 && a == 1 && sendDelay == 0){//Необходима для работы функции поиска новых СМС сообщений
	  Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Выводит результаты запроса СМС сообщения с указанным индексом
  }
}  

// =========================== MAIN =========================
int main(void) {
main_init(); // Инициализация МК

delay_ms(5000);
GPIOB->BSRR |= GPIO_BSRR_BS5;//Запуск simcom
delay_ms(1000);
GPIOB->BRR |= GPIO_BRR_BR5;
delay_ms(90000);//Ожидание инициализации модуля передачи данных 
Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//отправка ответа от simcom в МК

SMSMessageFormatcommand[8] = '1';
Send_command_from_uart_to_simcom(SMSMessageFormatcommand, 10);//Установка формата сообщений в TextMod
delay_ms(5000);//Ожидание ответа от simcom
Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Отправка ответа от simcom в МК

StartSocketService();//Вызов функции стартового конфигурирования сетевого интерфейса модуля передачи данных

uint8_t statusReady[7] = {'R', 'e', 'a', 'd', 'y', 0x0D, 0x1A};//Сообщение о готовности к работе и завершении конфигурирования
Send_command_from_simcom_to_uart(statusReady, 7);//Отправка сообщения о готовности к работе и завершении конфигурирования


// ========================= ГЛАВНЫЙ ЦИКЛ =========================

while (1) {
	
	if (a == 1 && sendDelay == 0){
		SMS_search(responseBuffer, responseIndex);
		Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Отправка сообщений от simcom в МК
	}
	if (b == 1 && sendDelay == 0){
		if (Command_check(commandBuffer, commandIndex) == 0){//Проверка на пользовательские команды 
			Send_command_from_uart_to_simcom(commandBuffer, commandIndex);//Отправка команд от МК к simcom
		}
	}
	
}
}

// КОНЕЦ