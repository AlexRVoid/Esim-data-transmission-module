#define    DWT_CYCCNT    *(volatile uint32_t *)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t *)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t *)0xE000EDFC
#define BUFFER_SIZE_COMMAND 500
#define BUFFER_SIZE_RESPONSE 1000
#define MAX_SMS_LENGTH 170

// ================== �������� ���������� ���������� =============
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
volatile uint16_t Timer = 0;
volatile uint8_t i = 0;
volatile uint8_t x = 0, y = 0, a = 0, b = 0, sendDelay = 0;

// ������� �������� � ��� (����� �� 1 ���). ��������: �������� + 1 ���.
static __INLINE  void delay_us(uint32_t us) {
static volatile uint32_t n;
n =  us * 24; // SystemCoreClock/1000000 = 24
DWT_CYCCNT = 0; while (DWT_CYCCNT < n) ;
}

// ������� �������� � �� �� 1 �� �� 59 ���. (������ ������������)
static __INLINE void delay_ms(uint32_t ms) {
static volatile uint32_t n;
n =  ms * 24000; DWT_CYCCNT = 0; while (DWT_CYCCNT < n) ;
}

// ============== ������������ ��������� ���������� ==============

void Send_command_from_simcom_to_uart(volatile uint8_t command[], volatile uint8_t commandSize){//������� �������� ������ � simcom � ��
	for (i = 0; i < commandSize; i++){
		while ((USART2->SR & USART_SR_TXE) == 0) {};//�������� �������� �� ����� � ����� �� �� ��������� ������ 
		USART2->DR = command[i];
	}
	responseIndex = 0;
	a = 0;
}

void Send_command_from_uart_to_simcom(volatile uint8_t command[], volatile uint8_t commandSize){//������� �������� ������ � �� � simcom
	
	for (i = 0; i < commandSize; i++){
		while ((USART1->SR & USART_SR_TXE) == 0) {};//�������� �������� �� ����� � ����� �� simcom ��������� ������ 
		USART1->DR = command[i];
	}
	commandIndex = 0;
	b = 0;
	
}

void USART1_IRQHandler() {
	
	static u8 ch1;
	ch1 = USART1->DR; 
	
	responseBuffer[responseIndex++] = ch1; // ��������� ������ � ������ �������
	
	sendDelay = 18;
	a = 1;
	
}

void USART2_IRQHandler() {
	static u8 ch2;
	ch2 = USART2->DR; 
	
	commandBuffer[commandIndex++] = ch2; // ��������� ������ � ������ �������
	
	sendDelay = 4;
	b = 1;
	
}  

//================================================================



void StartSocketService(){ //������������� ��� ������ ��� ������ � ����� � TCP/IP
	
	modeToRetrieveDataCommand[12] = '0';//��������� ��������������� ������ ������������������ �� TCP
	Send_command_from_uart_to_simcom(modeToRetrieveDataCommand, 14);//�������� �� ������� � simcom
	delay_ms(5000);//�������� ������� ������� 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//����� ���������� ���������� ������� 
	
	addIPHeaderCommand[11] = '1';//��������� IP ������ �������: "+IPD(data length)", ��� ������ �������� ������ �� TCP
	Send_command_from_uart_to_simcom(addIPHeaderCommand, 13);//�������� �� ������� � simcom
	delay_ms(5000);//�������� ������� ������� 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//����� ���������� ���������� ������� 
	
	showRemoteIPAddressAndPortCommand[11] = '1';//��������� ������ IP � ����� ��� ����� ������ �� TCP �������: "RECV FROM:<IP ADDRESS>:<PORT>"
	Send_command_from_uart_to_simcom(showRemoteIPAddressAndPortCommand, 13);//�������� �� ������� � simcom
	delay_ms(5000);//�������� ������� ������� 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//����� ���������� ���������� ������� 
	
	Send_command_from_uart_to_simcom(startSocketServiceCommand, 11);//�������� �� ������� �������� ������� ������� � simcom
	delay_ms(5000);//�������� ������� ������� 
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//����� ���������� ���������� ������� 
}

void SMS_search(volatile uint8_t command[], volatile uint8_t commandSize){//������� ������ ��� ��������� � ������ �� �� ��
	commandProcessing = 1;//���� ��������� ��� ���������
	
	for (i = 0; i < commandSize; i++){
		
		if(command[i] == '+'){//����� ���������� ������� ����������� � ��� ���������
			x = 0;
			
			while (command[i] == newSMSReferenceResponse[x]){//�������� ����������� � ��� ���������
				i++;
				x++;
				
				if (x == 6) {//� ������ ���������� ����������� � ��� ��������� ������� ���������� ���������
					
					while(command[i] != ','){//����� ������� ��� ���������
						i++;
					}
					i++;
					for (y = 8; command[i] != 0x0D; i++, y++){
						readSMSCommand[y] = command[i];//������������ �� ������� ��� ������ ��� ��������� 
					}
					readSMSCommand[y] = 0x0D;
					Send_command_from_uart_to_simcom(readSMSCommand, y+1);//�������� �� ������� 
				}
			}
		}
	}
	commandProcessing = 0;
}

uint8_t Send_SMS(volatile uint8_t phoneNum[], volatile uint8_t messageLength){ //�������� ��� ��������� �� ��������� �����
	a = 0;
	responseIndex = 0;
	for(i = 0; i < 12; i++){//���������� ������ �������� � �� � ������� 
		sendSMSCommand[i+9] = phoneNum[i];
	}
	sendSMSCommand[21] = '\"';
	sendSMSCommand[22] = 0x0D;
	responseIndex = 0;
	Send_command_from_uart_to_simcom(sendSMSCommand, 23);//�������� �� ������� � simcom
	sendSMSCommandResponseDelay = 50;//�������� ������� �������
	while((a == 0 && sendDelay != 0) || sendSMSCommandResponseDelay != 0){}
	if (a == 0){//� ������ ���������� ������� ��������� ��������� �� ������, ����� �� �������
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 'E';return 0;
	}
	x = 0;
	
	while(x < responseIndex){
		
		while(responseBuffer[x] == 62){//����� ������� '>' � ������ �� simcom , ������ '>' ������ ���������� ������ ������ ��� ���������
			
			
			Send_command_from_uart_to_simcom(message, messageLength);//�������� � simcom ������ ���������
			delay_ms(5000);
			USART1->DR = 0x1A;//�������� ��� ��������� ��������
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';//��������� �� �������� ��� ���������
			return 1;
			
		}
		x++;
	}
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 'E';// ��� ���������� ������� ������ ������ ��������� ������� ��������� ������
	return 0;
	
}

uint8_t Command_check(volatile uint8_t command[], volatile uint8_t commandSize){// �������� ���������������� ������
	i = 0;
	uint8_t SMSResponseIndex = 0;
	while((command[i] == SMSCommandReference[i]) && (i < 3)){//���������������� ������� ��� �������� ��� ��������� 
		SMSResponseIndex++;
		i++;
		if (i == 3){//� ������ ������� �������� ����������� �������� ��� ��������� �� ��������� �����
			b = 0;
			volatile uint8_t phone[12];//������ ����� �������� �� ������� ����� ����������� ���������
			//SMS,n:+79963438273,m:...
			//������������ ������ ��������� 170 ��������, ������ ��������
			uint8_t x = 0;
			while(command[x] != 'n' && x < commandSize){//����� ������ ������ ��������
				x++;
			}
			x++;
			x++;
			for(i = 0; i <= 12; i++, x++){//������ ������ ��������
				phone[i] = command[x];
			}
			while(command[x] != 'm' && x < commandSize){//����� ������ ������ ��� ���������
				x++;
			}
			x++;
			x++;
			volatile uint8_t SMSLength = commandSize - x;
			if (SMSLength <= MAX_SMS_LENGTH){//�������� ������ ��� ���������
				for (i = 0; x < commandSize; i++, x++){//������ ������ ��� ���������
					message[i] = command[x];
				}
				Send_SMS(phone, SMSLength);//����� ������� �������� ��� ���������
				commandIndex = 0;
				return 1;
			}
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'E';//��� ���������� ������ ��������� �������� ��������� �� ������
			commandIndex = 0;
			return 1;	
			
		}
	}	
	
	i = 0;
	uint8_t TCPResponseIndex = 0;
	while((command[i] == TCPCommandReference[i]) && (i < 3)){//���������������� ������� ��� �������� ������ �� TCP/IP
		TCPResponseIndex++;
		i++;
		if (i == 3){//� ������ ������� �������� ����������� �������� ������ �� ��������� �����
			b = 0;
			volatile uint8_t ip[15];//������ ip ����� ����������
			volatile uint8_t ipLenght = 0;//������ ������ ip ������ 
			volatile uint8_t port[5];//������ ���� ����������
			volatile uint8_t portLength = 0;//������ ������ ����� ������
			//TCP,ip:217.71.129.139,p:4742,m:...
			//������������ ������ ��������� 170 ��������, ������ ��������
			uint8_t x = 0;
			while(command[x] != 'i' && x < commandSize){//����� ������ ip ������
				x++;
			}
			x++;
			x++;
			x++;
			for(i = 0; command[x] != ',' && i < 15; i++, x++, ipLenght++){//������ ip ������
				ip[i] = command[x];
			}
			while(command[x] != 'p' && x < commandSize){//����� ������ �����
				x++;
			}
			x++;
			x++;
			for(i = 0; command[x] != ',' && i < 5; i++, x++, portLength++){//����� ����� 
				port[i] = command[x];
			}
			
			while(command[x] != 'm' && x < commandSize){//����� ������ ��������� 
				x++;
			}
			x++;
			x++;
			volatile uint8_t SMSLength = commandSize - x;
				
			if (SMSLength <= MAX_SMS_LENGTH){//�������� ������ ���������
				for (i = 0; x < commandSize; i++, x++){
					message[i] = command[x];
				}
				
				Send_TCP(ip, ipLenght, port, portLength, SMSLength);//����� ������� �������� ������ �� TCP/IP
				commandIndex = 0;
				return 1;
			}
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'E';//��� ���������� ������ ��������� �������� ��������� �� ������
			commandIndex = 0;
			return 1;	
		}
	}
	
	uint8_t callResponseIndex = 0;
	while((command[i] == callCommandReference[i]) && (i < 4)){//���������������� ������� ������ �� ��������� �����
		callResponseIndex++;
		i++;
		if (i == 4){
			b = 0;
			volatile uint8_t phone[12];//������ ����� ��������
			//CALL,n:+79963438273
			uint8_t x = 0;
			while(command[x] != 'n' && x < commandSize){//���� ������ ������ �������� 
				x++;
			}
			x++;
			x++;
			for(i = 0; i <= 12; i++, x++){//���������� ����� �������� 
				phone[i] = command[x];
			}
			VoiceCall(phone);//����� ������� ���������� ������
			commandIndex = 0;
			return 1;
		}
	}
	
	uint8_t SIMCardSwitchResponseIndex = 0;
	while((command[i] == SIMCardSwitchReference[i]) && (i < 9)){//���������������� �������� ������ �������� ��� �����
		SIMCardSwitchResponseIndex++;
		i++;
		if (i == 9){
			b = 0;
			volatile uint8_t simNum;//������ ����� ��� ����� ��� ������������
			//SIMSwitch,x
			//x - ����� �������� (0-1)
			//0 - SIM1, 1 - SIM2
			uint8_t x = 0;
			while(command[x] != ',' && x < commandSize){//����� ������ ��� �����
				x++;
			}
			x++;
			simNum = command[x];//������ ������ ��� �����
			SIMCardSwitch(simNum);//����� ������� ������������ �������� ��� ����� 
			commandIndex = 0;
			return 1;
		}
	}
	
	
	return 0;
}


void TCP_IP_Connect(volatile uint8_t ip[], volatile uint8_t ipLenght, volatile uint8_t port[], volatile uint8_t portLength){//������� ����������� �� TCP/IP
	// Send_command_from_uart_to_simcom(startSocketServiceCommand, 11);//������������ �������� ������� ��������� ������ �������
	// delay_ms(10000);
	// Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	a = 0;
	uint8_t x = 20;
	responseIndex = 0;
	for(i = 0; i < ipLenght; i++, x++){//������ ����������� IP � �� �������
		TCPConnectCommand[x] = ip[i];
	}
	
	TCPConnectCommand[x] = '\"';
	x++;
	TCPConnectCommand[x] = ',';
	x++;
	
	for(i = 0; i < portLength; i++, x++){//������ ����� � �� �������
		TCPConnectCommand[x] = port[i];
	}
	
	TCPConnectCommand[x] = 0x0D;//���������� �� �������, ������ CR
	responseIndex = 0;
	Send_command_from_uart_to_simcom(TCPConnectCommand, x + 1);//�������� �� �������� �� simcom
	delay_ms(10000);//�������� ������ �� simcom
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//�������� ������ �� ��
}

void Send_TCP(volatile uint8_t ip[], volatile uint8_t ipLenght, volatile uint8_t port[], volatile uint8_t portLength, volatile uint8_t messageLength){//������� �������� ������ �� TCP/IP
	TCP_IP_Connect(ip, ipLenght, port, portLength);//����� ������� ����������� �� TCP
	Send_command_from_uart_to_simcom(sendTCPMessageCommand, 13);//�������� �� ������� ��� �������� ������ �� TCP � simcom
	a = 0;
	responseIndex = 0;
	sendTCPCommandResponseDelay = 10;
	while((a == 0 && sendDelay != 0) || sendTCPCommandResponseDelay != 0){}//�������� ������ �� simcom � ���������� � �������� ������ �� TCP 
	if (a == 0){//� ������ ���������� ������ ���������� ��������� �� ������, � ������� �� �������
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 'E';
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 0x0D;
		return;
	}
	x = 0;
	while(x < responseIndex){
		while(responseBuffer[x] == 62){//����� ������� ">" � ������
			
			Send_command_from_uart_to_simcom(message, messageLength);//�������� � simcom ������ ��� �������� �� TCP 
			delay_ms(5000);
			USART1->DR = 0x1A;//���������� ����� ������ � �������� ������ �� ��������� �����
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';//��������� �� �������� ��������
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 0x0D;
			return;
			
		}
		x++;
	}
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//�������� � �� ������ �� simcom
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 'E';//����������� ������ �������� ������
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 0x0D;
	return;
	
	
	
}

void VoiceCall(volatile uint8_t phoneNum[]){ //��������� ����� �� ��������� �����
	for(i = 0; i < 12; i++){//���������� ������ �������� � �� � ������� 
		callCommand[i+3] = phoneNum[i];
	}
	callCommand[15] = ';';
	callCommand[16] = 0x0D;
	Send_command_from_uart_to_simcom(callCommand, 17);//�������� �� ������� � simcom
}

void SIMCardSwitch(volatile uint8_t simNum){ //����� �������� ��� �����.
	SIMCardSwitchCommand[13] = simNum;//���������� ������ ����� ��� ����� � AT ������� 
	Send_command_from_uart_to_simcom(SIMCardSwitchCommand, 15);//�������� �� ������� � simcom
}



// �����
