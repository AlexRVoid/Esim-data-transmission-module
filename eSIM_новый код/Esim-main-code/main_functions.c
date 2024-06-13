#define    DWT_CYCCNT    *(volatile uint32_t *)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t *)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t *)0xE000EDFC
#define BUFFER_SIZE_COMMAND 500
#define BUFFER_SIZE_RESPONSE 1000
#define MAX_SMS_LENGTH 170

// ================== Описание глобальных переменных =============
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

// Функция задержки в мкс (можно от 1 мкс). Получите: задержка + 1 мкс.
static __INLINE  void delay_us(uint32_t us) {
static volatile uint32_t n;
n =  us * 24; // SystemCoreClock/1000000 = 24
DWT_CYCCNT = 0; while (DWT_CYCCNT < n) ;
}

// Функция задержки в мс от 1 мс до 59 сек. (дальше переполнение)
static __INLINE void delay_ms(uint32_t ms) {
static volatile uint32_t n;
n =  ms * 24000; DWT_CYCCNT = 0; while (DWT_CYCCNT < n) ;
}

// ============== Подпрограммы обработки прерывания ==============

void Send_command_from_simcom_to_uart(volatile uint8_t command[], volatile uint8_t commandSize){//Функция отправки данных с simcom в МК
	for (i = 0; i < commandSize; i++){
		while ((USART2->SR & USART_SR_TXE) == 0) {};//Проверка свободен ли канал и готов ли МК принимать данные 
		USART2->DR = command[i];
	}
	responseIndex = 0;
	a = 0;
}

void Send_command_from_uart_to_simcom(volatile uint8_t command[], volatile uint8_t commandSize){//Функция отправки данных с МК в simcom
	
	for (i = 0; i < commandSize; i++){
		while ((USART1->SR & USART_SR_TXE) == 0) {};//Проверка свободен ли канал и готов ли simcom принимать данные 
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

//================================================================



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

void SMS_search(volatile uint8_t command[], volatile uint8_t commandSize){//Функция поиска СМС сообщений и вывода их на мк
	commandProcessing = 1;//Флаг обработки СМС сообщения
	
	for (i = 0; i < commandSize; i++){
		
		if(command[i] == '+'){//Поиск начального символа уведомления о СМС сообщении
			x = 0;
			
			while (command[i] == newSMSReferenceResponse[x]){//Проверка уведомления о СМС сообщении
				i++;
				x++;
				
				if (x == 6) {//В случае нахождения уведомления о СМС сообщении выводит полученное сообщение
					
					while(command[i] != ','){//Поиск индекса СМС сообщения
						i++;
					}
					i++;
					for (y = 8; command[i] != 0x0D; i++, y++){
						readSMSCommand[y] = command[i];//Формирование АТ команды для вывода СМС сообщения 
					}
					readSMSCommand[y] = 0x0D;
					Send_command_from_uart_to_simcom(readSMSCommand, y+1);//Отправка АТ команды 
				}
			}
		}
	}
	commandProcessing = 0;
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
	Send_command_from_uart_to_simcom(sendSMSCommand, 23);//Отправка АТ команды в simcom
	sendSMSCommandResponseDelay = 50;//ожидание отклика команды
	while((a == 0 && sendDelay != 0) || sendSMSCommandResponseDelay != 0){}
	if (a == 0){//В случае отсутствия отклика отправить сообщение об ошибке, выйти из функции
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 'E';return 0;
	}
	x = 0;
	
	while(x < responseIndex){
		
		while(responseBuffer[x] == 62){//Поиск символа '>' в ответе от simcom , символ '>' служит указателем начала текста СМС сообщения
			
			
			Send_command_from_uart_to_simcom(message, messageLength);//Отправка в simcom текста сообщения
			delay_ms(5000);
			USART1->DR = 0x1A;//Отправка СМС сообщения адресату
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';//Сообщение об отправке СМС сообщения
			return 1;
			
		}
		x++;
	}
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 'E';// При отсутствии символа начала текста сообщения вывести сообщение ошибки
	return 0;
	
}

uint8_t Command_check(volatile uint8_t command[], volatile uint8_t commandSize){// Проверка пользовательских команд
	i = 0;
	uint8_t SMSResponseIndex = 0;
	while((command[i] == SMSCommandReference[i]) && (i < 3)){//Пользовательская команда для отправки СМС сообщения 
		SMSResponseIndex++;
		i++;
		if (i == 3){//В случае удачной проверки выполняется отправка СМС сообщения на указанный номер
			b = 0;
			volatile uint8_t phone[12];//хранит номер телефона на который будет отправленно сообщение
			//SMS,n:+79963438273,m:...
			//максимальная длинна сообщения 170 символов, только латиница
			uint8_t x = 0;
			while(command[x] != 'n' && x < commandSize){//поиск начала номера телефона
				x++;
			}
			x++;
			x++;
			for(i = 0; i <= 12; i++, x++){//Запись номера телефона
				phone[i] = command[x];
			}
			while(command[x] != 'm' && x < commandSize){//Поиск начала текста СМС сообщения
				x++;
			}
			x++;
			x++;
			volatile uint8_t SMSLength = commandSize - x;
			if (SMSLength <= MAX_SMS_LENGTH){//Проверка длинны СМС сообщения
				for (i = 0; x < commandSize; i++, x++){//Запись текста СМС сообщения
					message[i] = command[x];
				}
				Send_SMS(phone, SMSLength);//Вызов функции отправки СМС сообщения
				commandIndex = 0;
				return 1;
			}
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'E';//При привышении длинны сообщения отправка сообщения об ошибке
			commandIndex = 0;
			return 1;	
			
		}
	}	
	
	i = 0;
	uint8_t TCPResponseIndex = 0;
	while((command[i] == TCPCommandReference[i]) && (i < 3)){//Пользовательская команда для отправки данных по TCP/IP
		TCPResponseIndex++;
		i++;
		if (i == 3){//В случае удачной проверки выполняется отправка данных на указанный адрес
			b = 0;
			volatile uint8_t ip[15];//Хранит ip адрес получателя
			volatile uint8_t ipLenght = 0;//Хранит длинну ip адреса 
			volatile uint8_t port[5];//Хранит порт получателя
			volatile uint8_t portLength = 0;//Хранит длинну порта адреса
			//TCP,ip:217.71.129.139,p:4742,m:...
			//максимальная длинна сообщения 170 символов, только латиница
			uint8_t x = 0;
			while(command[x] != 'i' && x < commandSize){//Поиск начала ip адреса
				x++;
			}
			x++;
			x++;
			x++;
			for(i = 0; command[x] != ',' && i < 15; i++, x++, ipLenght++){//Запись ip адреса
				ip[i] = command[x];
			}
			while(command[x] != 'p' && x < commandSize){//Поиск начала порта
				x++;
			}
			x++;
			x++;
			for(i = 0; command[x] != ',' && i < 5; i++, x++, portLength++){//Зпись порта 
				port[i] = command[x];
			}
			
			while(command[x] != 'm' && x < commandSize){//Поиск начала сообщения 
				x++;
			}
			x++;
			x++;
			volatile uint8_t SMSLength = commandSize - x;
				
			if (SMSLength <= MAX_SMS_LENGTH){//Проверка длинны сообщения
				for (i = 0; x < commandSize; i++, x++){
					message[i] = command[x];
				}
				
				Send_TCP(ip, ipLenght, port, portLength, SMSLength);//Вызов функции отправки данных по TCP/IP
				commandIndex = 0;
				return 1;
			}
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'E';//При привышении длинны сообщения отправка сообщения об ошибке
			commandIndex = 0;
			return 1;	
		}
	}
	
	uint8_t callResponseIndex = 0;
	while((command[i] == callCommandReference[i]) && (i < 4)){//Пользовательская команда звонка на указанный номер
		callResponseIndex++;
		i++;
		if (i == 4){
			b = 0;
			volatile uint8_t phone[12];//Хранит номер телефона
			//CALL,n:+79963438273
			uint8_t x = 0;
			while(command[x] != 'n' && x < commandSize){//Ищет начало номера телефона 
				x++;
			}
			x++;
			x++;
			for(i = 0; i <= 12; i++, x++){//Записывает номер телефона 
				phone[i] = command[x];
			}
			VoiceCall(phone);//Вызов функции голосового вызова
			commandIndex = 0;
			return 1;
		}
	}
	
	uint8_t SIMCardSwitchResponseIndex = 0;
	while((command[i] == SIMCardSwitchReference[i]) && (i < 9)){//Пользовательская комманда выбора основной сим карты
		SIMCardSwitchResponseIndex++;
		i++;
		if (i == 9){
			b = 0;
			volatile uint8_t simNum;//Хранит номер сим карты для переключения
			//SIMSwitch,x
			//x - номер симкарты (0-1)
			//0 - SIM1, 1 - SIM2
			uint8_t x = 0;
			while(command[x] != ',' && x < commandSize){//Поиск номера сим карты
				x++;
			}
			x++;
			simNum = command[x];//Запись номера сим карты
			SIMCardSwitch(simNum);//Вызов функции переключения основной сим карты 
			commandIndex = 0;
			return 1;
		}
	}
	
	
	return 0;
}


void TCP_IP_Connect(volatile uint8_t ip[], volatile uint8_t ipLenght, volatile uint8_t port[], volatile uint8_t portLength){//Функция подключения по TCP/IP
	// Send_command_from_uart_to_simcom(startSocketServiceCommand, 11);//Дублирование отправки команды открытися службы сокетов
	// delay_ms(10000);
	// Send_command_from_simcom_to_uart(responseBuffer, responseIndex);
	a = 0;
	uint8_t x = 20;
	responseIndex = 0;
	for(i = 0; i < ipLenght; i++, x++){//запись полученного IP в АТ команду
		TCPConnectCommand[x] = ip[i];
	}
	
	TCPConnectCommand[x] = '\"';
	x++;
	TCPConnectCommand[x] = ',';
	x++;
	
	for(i = 0; i < portLength; i++, x++){//Запись порта в АТ команду
		TCPConnectCommand[x] = port[i];
	}
	
	TCPConnectCommand[x] = 0x0D;//завершение АТ команды, символ CR
	responseIndex = 0;
	Send_command_from_uart_to_simcom(TCPConnectCommand, x + 1);//Отправка АТ комманды на simcom
	delay_ms(10000);//Ожидание ответа от simcom
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//Отправка ответа на МК
}

void Send_TCP(volatile uint8_t ip[], volatile uint8_t ipLenght, volatile uint8_t port[], volatile uint8_t portLength, volatile uint8_t messageLength){//Функция отправки данных по TCP/IP
	TCP_IP_Connect(ip, ipLenght, port, portLength);//Вызов функции подключения по TCP
	Send_command_from_uart_to_simcom(sendTCPMessageCommand, 13);//Отправка АТ команды для отправки данных по TCP в simcom
	a = 0;
	responseIndex = 0;
	sendTCPCommandResponseDelay = 10;
	while((a == 0 && sendDelay != 0) || sendTCPCommandResponseDelay != 0){}//Ожидание ответа от simcom о готовности к отправки данных по TCP 
	if (a == 0){//В случае отсутствия ответа возвращает сообщение об ошибке, и выходит из функции
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 'E';
		while ((USART2->SR & USART_SR_TXE) == 0) {};
		USART2->DR = 0x0D;
		return;
	}
	x = 0;
	while(x < responseIndex){
		while(responseBuffer[x] == 62){//Поиск символа ">" в ответе
			
			Send_command_from_uart_to_simcom(message, messageLength);//Отправка в simcom данных для отправки по TCP 
			delay_ms(5000);
			USART1->DR = 0x1A;//Завершение ввода данных и отправка данных на указанный адрес
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 'S';//сообщение об успешной отправке
			while ((USART2->SR & USART_SR_TXE) == 0) {};
			USART2->DR = 0x0D;
			return;
			
		}
		x++;
	}
	Send_command_from_simcom_to_uart(responseBuffer, responseIndex);//отправка в МК ответа от simcom
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 'E';//Сообщениеоб ошибке отправки данных
	while ((USART2->SR & USART_SR_TXE) == 0) {};
	USART2->DR = 0x0D;
	return;
	
	
	
}

void VoiceCall(volatile uint8_t phoneNum[]){ //Голосовой вызов на указанный номер
	for(i = 0; i < 12; i++){//Добавление номера телефона к АТ к команде 
		callCommand[i+3] = phoneNum[i];
	}
	callCommand[15] = ';';
	callCommand[16] = 0x0D;
	Send_command_from_uart_to_simcom(callCommand, 17);//Отправка АТ команды в simcom
}

void SIMCardSwitch(volatile uint8_t simNum){ //Смена основной сим карты.
	SIMCardSwitchCommand[13] = simNum;//Добавление номера слота сим карты к AT команде 
	Send_command_from_uart_to_simcom(SIMCardSwitchCommand, 15);//Отправка АТ команды в simcom
}



// КОНЕЦ
