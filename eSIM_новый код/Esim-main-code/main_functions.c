#define    DWT_CYCCNT    *(volatile uint32_t *)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t *)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t *)0xE000EDFC

// ������� ��� ������������� ����������� �������� � �������������� DWT
// �� ���������, ��� ������ ���������� ���� ������ ������� ������� �������

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

// �����
