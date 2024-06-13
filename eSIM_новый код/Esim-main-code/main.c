/*���� ��������� ��������: 13 ���� 2024 ����
��������: �� "���"
�����: Blue Pill
���������������: STM32F103C6T6
���� ����������������: �� + ������ � ���������� + CMSIS
�����: ������ �.�, ������ �.�.

������� � ������������ ����������� ������� � USART2(115200 ���) �� USART1(115200 ���).
������ ����������� PC13.

============ ����� �������� ============
USART1: PA10 = RX1
USART2: PA3 = RX2
============ ������ �������� ============
USART1: PA9 = TX1
USART2: PA2 = TX2
�������� �����: PB5 = ��������� ������ �������� ������
�������� �����: PB6 = ������������ ������ �������� ������

*/                       

#include "stm32f10x.h"
#include "main_functions.c" 
#include "main_init.c"


// =================== TIM2 0.5 ���. ===================
void TIM2_IRQHandler() {
  if ((TIM2->SR & TIM_SR_UIF) != 0) { // ���������� �� ������������
    TIM2->SR &= ~ TIM_SR_UIF; 
    Timer++; // ��� ������� �������
    GPIOC->ODR ^= GPIO_ODR_ODR13; // ������ ����������� PC8
  }
  if (sendDelay != 0){
	  sendDelay--;//�������� ��� ��������� ������ �� simcom ��� stm
  }
  
  if(sendSMSCommandResponseDelay != 0){
	  sendSMSCommandResponseDelay--;//�������� ��� ��������� ������ �� simcom ��� �������� ��� ���������
  }
  
  if(sendTCPCommandResponseDelay != 0){
	sendTCPCommandResponseDelay--;//�������� ��� ��������� ������ �� simcom ��� �������� ������ �� TCP
  }
  
  if (commandProcessing == 1 && a == 1 && sendDelay == 0){//���������� ��� ������ ������� ������ ����� ��� ���������
	  Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//������� ���������� ������� ��� ��������� � ��������� ��������
  }
}  

// =========================== MAIN =========================
int main(void) {
main_init(); // ������������� ��

delay_ms(5000);
GPIOB->BSRR |= GPIO_BSRR_BS5;//������ simcom
delay_ms(1000);
GPIOB->BRR |= GPIO_BRR_BR5;
delay_ms(90000);//�������� ������������� ������ �������� ������ 
Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//�������� ������ �� simcom � ��

SMSMessageFormatcommand[8] = '1';
Send_command_from_uart_to_simcom(SMSMessageFormatcommand, 10);//��������� ������� ��������� � TextMod
delay_ms(5000);//�������� ������ �� simcom
Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//�������� ������ �� simcom � ��

StartSocketService();//����� ������� ���������� ���������������� �������� ���������� ������ �������� ������

uint8_t statusReady[7] = {'R', 'e', 'a', 'd', 'y', 0x0D, 0x1A};//��������� � ���������� � ������ � ���������� ����������������
Send_command_from_simcom_to_uart(statusReady, 7);//�������� ��������� � ���������� � ������ � ���������� ����������������


// ========================= ������� ���� =========================

while (1) {
	
	if (a == 1 && sendDelay == 0){
		SMS_search(responseBuffer, responseIndex);
		Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//�������� ��������� �� simcom � ��
	}
	if (b == 1 && sendDelay == 0){
		if (Command_check(commandBuffer, commandIndex) == 0){//�������� �� ���������������� ������� 
			Send_command_from_uart_to_simcom(commandBuffer, commandIndex);//�������� ������ �� �� � simcom
		}
	}
	
}
}

// �����