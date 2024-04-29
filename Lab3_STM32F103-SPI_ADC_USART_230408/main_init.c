// ========= ������������� ���������������� =========
static __INLINE void main_init() {
// ����� ����������� ��������, 2 ����� �������� ��� FLASH, ��. PM0075
FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // HCLK = SYSCLK
RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // PCLK1 = SYSCLK / 2 (APB1)
RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // PCLK2 = SYSCLK (APB2)
RCC->CFGR |= RCC_CFGR_SW_PLL; // �������� ������ �� PLL
RCC->CFGR |= RCC_CFGR_PLLMULL9; // SYSCLK = 72 MHz
RCC->CFGR |= RCC_CFGR_PLLSRC_HSE; // ������������ PLL �� ������
RCC->CR |= RCC_CR_HSEON; //��������� ��������� HSE
while (!(RCC->CR & RCC_CR_HSERDY)) {};
RCC->CR |= RCC_CR_PLLON; // ��������� PLL
while ((RCC->CR & RCC_CR_PLLRDY) == 0) {};
// ������� ����� PLL ������ ���������� ������������
while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1) {};
//RCC->CR &= ~RCC_CR_HSION; // ��������� ���������� RC-���������

//====================== DWT ======================
SCB_DEMCR |= 0x01000000; // ��������� ������������ �������
DWT_CYCCNT = 0; // �������� �������� �������� ��������
DWT_CONTROL |= 1; // ��������� �������  

//====================== ����� ====================== 
 // �������� ������������ ����� GPIOA, GPIOB, GPIOC 
RCC->APB2ENR = (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN);
RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;  

//================= ������ ������ "A" ===========================
	//======================== ��� ========================
	GPIOA->CRL &= ~GPIO_CRL_MODE0; // PA0 = ���� 
	GPIOA->CRL &= ~GPIO_CRL_MODE1; // PA1 = ���� 
	GPIOA->CRL &= ~GPIO_CRL_CNF0; // PA0 = ����������
	GPIOA->CRL &= ~GPIO_CRL_CNF1; // PA1 = ����������

	//====================== SPI1 ====================== 
	// PA4 - CS (��������� ��� ������ � �������� ������)
	// PA5 - SPI1: CLK    PA7 - SPI1: MOSI
	// CS ����������� ����� PA4 ��� ����� open-drain 10 mhz
	GPIOA->CRL &= ~GPIO_CRL_MODE4;
	GPIOA->CRL |= GPIO_CRL_MODE4_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF4;
	// CLK ����������� ����� PA5 � ��� �������������� ����� open-drain 10 mhz
	GPIOA->CRL &= ~GPIO_CRL_MODE5; 
	GPIOA->CRL |= GPIO_CRL_MODE5_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF5;
	GPIOA->CRL |= GPIO_CRL_CNF5;
	// MOSI ����������� ����� PA7 � ��� �������������� ����� open-drain 10 mhz
	GPIOA->CRL &= ~GPIO_CRL_MODE7; 
	GPIOA->CRL |= GPIO_CRL_MODE7_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF7;
	GPIOA->CRL |= GPIO_CRL_CNF7;

	//====================== USART1 ====================== 
	// PA9 (TX1) AFIO Push-Pull, 10MHz. PA10 (RX1) HiZ, 10MHz
	// ������� ������������� ���� ��� � "00", ��� ������ ����������� !
	GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9 | GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
	// ����� ������ ���� ������������� � '1'
	GPIOA->CRH |= (GPIO_CRH_MODE9_0 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF10_0); 


//================= ������ ������ "C" ===========================
	// pin 13 ������� ��������� �� �����
	GPIOC->CRH &= ~GPIO_CRH_MODE13; // ����� �������� MODE
	// ��������� ������: ����. �����, ������� ����������� �� 10 ���
	GPIOC->CRH |= GPIO_CRH_MODE13; 
	// ��������� ������: ����. ����� - Push-pull
	GPIOC->CRH &= ~GPIO_CRH_CNF13;

//====================== USART1 ====================== 
RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // �������� ������������ USART1
// PA9 (TX1) AFIO Push-Pull, 10MHz. PA10 (RX1) HiZ, 10MHz
USART1->BRR = 7500; // PCLK2 / Baud = 72000000 / 9600 ���
// �������� USART, ���������� � ��������
USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
USART1->CR1 |= USART_CR1_RXNEIE; // ��������� ���������� �� ����� RX
// ��������� �� ���������: 8 ���. ���, 1 �������� ���, �������� �������� ���
// ��������� ���������� UART1 � ����������� ����������
NVIC_EnableIRQ(USART1_IRQn);

//====================== SPI1 ====================== 
RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;     // ��������� ������������ SPI1
	
SPI1->CR1 |= SPI_CR1_BR;                //Baud rate = PCLK1/256 	(140 KHz)
SPI1->CR1 &= ~SPI_CR1_CPOL;             //�� �������� CLK 			CPOL = 0;
SPI1->CR1 &= ~SPI_CR1_CPHA;             //�� ��������� ������    	CPHA = 0;
SPI1->CR1 |= SPI_CR1_DFF;               //16 bit data
SPI1->CR1 &= ~SPI_CR1_LSBFIRST;         //������� ��� ������		MSB 
SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;  //Software slave management & Internal slave select (NSS)

SPI1->CR1 |= SPI_CR1_MSTR;              //Mode Master
SPI1->CR1 |= SPI_CR1_SPE;                //Enable SPI1

//======================== ��� ========================
RCC->CFGR |= RCC_CFGR_ADCPRE_DIV8; // ������������ PCLK2 / 8 = 9 MHz
RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // ��������� ������������ ADC1	  
RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST; // ����� ADC1
RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST; // ����� ������ ADC1  

ADC1->CR1 &= ~ADC_CR1_DUALMOD; // ����������� ����� ������
ADC1->CR1 &= ~ADC_CR1_SCAN; // ����� ��� ������������	 

ADC1->CR2 &= ~ADC_CR2_CONT; // ����� ������������ ��������������
// �������������� �� ������� ��������� � ��� SWSTART
ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL; 

// ����� ������� ������ 239,5 �����
ADC1->SMPR2 |= ADC_SMPR2_SMP0;
 
ADC1->CR2 |= ADC_CR2_ADON; // �������� ADC1 
ADC1->CR2 |= ADC_CR2_RSTCAL; delay_us(10); // ��������� ����� ��������� ���������� 
while (ADC1->CR2 & ADC_CR2_RSTCAL) {}; // ��� ��������� ������	��������� 
ADC1->CR2 |= ADC_CR2_CAL; // ��������� ����������	   
while (ADC1->CR2 & ADC_CR2_CAL) {}; // ��� ��������� ����������

//====================== I2C ======================
	// ��������� ������������ I2C � GPIO    
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    // ��������� ������� PB6 (SCL) � PB7 (SDA) �� �������������� ������� (AF_OD)    
	GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7 | GPIO_CRL_MODE6 | GPIO_CRL_MODE7);
    GPIOB->CRL |= GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1;    
	GPIOB->ODR |= GPIO_ODR_ODR6 | GPIO_ODR_ODR7;
    // ��������� ��������� I2C ��� ������������ ����������� EEPROM
    I2C1->CR1 &= ~I2C_CR1_PE; // ���������� I2C ����� ����������    
	I2C1->CR2 |= (36 << 0);    // ������� ������������ I2C (36 ���)
    I2C1->CCR = 180;           // ��������� ������� I2C (100 ���)    
	I2C1->TRISE = 37;          // ��������� ������� ����������/����� (�������� ��� 100 ���)
    I2C1->CR1 |= I2C_CR1_PE;   // ��������� I2C

// ======================== TIM2 =======================
// ��������� ������������, PCLK1 = 12 ���, TIM2 ���������� x 2 = 24 ���
RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
TIM2->SMCR &= ~TIM_SMCR_SMS; // �������� ���������� ������������
TIM2->PSC = 720 - 1; // ��� (�����) ����� 10 ���, 100 KHz
TIM2->ARR = 50000 - 1; // ������������ ����� 0.5 ���.
TIM2->DIER |= TIM_DIER_UIE; // ��������� ���������� �� ������������ �������
NVIC_EnableIRQ(TIM2_IRQn); // ��������� ���������� � ����������� ����������
TIM2->CR1 = TIM_CR1_CEN; // ��������� ����
}  
// �����