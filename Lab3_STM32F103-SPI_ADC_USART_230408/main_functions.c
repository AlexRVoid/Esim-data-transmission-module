#define    DWT_CYCCNT    *(volatile uint32_t *)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t *)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t *)0xE000EDFC

// Функции для генерирования программной задержки с использованием DWT
// Не забывайте, что разные прерывания могу мешать точному отсчёту времени

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

// Функция преобразования беззнакового 16 битного числа в строку
void Uint16ToStr(uint16_t Number, uint8_t *pStr) {
static int8_t i;
for(i = 4; i != -1; i--) { pStr[i] = (Number % 10) + 0x30; Number /= 10; }
pStr[5] = 0; // конец строки
}

// Функция преобразования знакового 16 битного числа в строку
void Int16ToStr(int16_t Number, uint8_t *pStr) {
static int8_t i;
if (Number >=0) pStr[0] = '+'; 
  else { pStr[0] = '-'; Number = -Number; }
for(i = 5; i != 0; i--) { pStr[i] = (Number % 10) + 0x30; Number /= 10; }
pStr[6] = 0; // конец строки
}

// Функция преобразования беззнакового 32 битного числа в строку
// 0 .. 4 294 967 295  <CR> = 13, <LF> = 10
void Uint32ToStr(uint32_t Number, uint8_t *pStr) {
static int8_t i;
for(i = 9; i != -1; i--) { pStr[i] = (Number % 10) + 0x30; Number /= 10; }
pStr[10] = 0;
}

// Функция передачи символа по USART1 по готовности
void Tx1(uint8_t Symbol) {
while ((USART1->SR & USART_SR_TXE) == 0) {};
USART1->DR = Symbol;
}

// Функция ожидания и приёма символа по USART1
uint8_t Rx1() {
while ((USART1->SR & USART_SR_RXNE) == 0) {};
return USART1->DR;
}

// Функция передачи массива символов (строки) по USART1
// пока не встретится "0" байт
void Tx1Str(uint8_t *pStr) {
static uint8_t i;
i = 0; do Tx1( pStr[i++] ); while (pStr[i] != 0);
}

// Функция передачи массива символов (строки) CONST по USART1
// пока не встретится "0" байт
void Tx1StrConst(const uint8_t *pStr) {
static uint8_t i;
i = 0; do Tx1( pStr[i++] ); while (pStr[i] != 0);
}

// КОНЕЦ
