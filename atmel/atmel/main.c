//# define F_CPU 1000000UL



#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

const size_t rBufSize = 128;
const size_t rBufOffs = 0;
//PB0 - ������ � ������� ���������
//PB1 - ������� ������� ����
//PB2 - ������ � ������� �����
//PB3 - ������� ������� �����
//PB4 - ������ �� �������
//PB5 - ����� ���


size_t GetEIndex (int16_t idx) {
    return (size_t)((idx + (int16_t)rBufSize) % rBufSize);
}

uint8_t ERead(size_t idx){
    return eeprom_read_byte((size_t*)(rBufOffs + idx));
}

void EWrite(size_t idx,  uint8_t data){
    eeprom_write_byte((size_t*)(idx + rBufOffs), data) ;
}

void EClear(){
    for (int i = 0; i < rBufSize; i++){
        EWrite(GetEIndex(i), 0);
    }
}

void InitHead(size_t idx) {
    EWrite(idx, 0);
    EWrite(GetEIndex(idx - 1), 1);
}

int GetHeadIndex() {
    for(int i = 0; i < rBufSize; i++){
        if (ERead(i) == 0){
            return i;
        }
    }
    InitHead(0);
    return 0;
}

size_t GetDataIndex() {
    size_t hIdx = GetHeadIndex();
    return GetEIndex(hIdx - 1);
}

uint8_t CounterPlus(){
    size_t dIdx = GetDataIndex();
    uint8_t data = ERead(dIdx) + 1;
    if (data > maxCntVal) {
        InitHead(GetHeadIndex() + 1);
        return 1;
    } else {
        EWrite(dIdx, data);
    }
    return data;
}

inline void SetupADCSRA(){
	//         ADEN           - ������ ������� �� ���
	//         |ADSC          - ������ ���
	//         ||ADATE        - ��������� �� ������������� �������� ADTS[2:0]
	//         |||ADIF        - ���� ���������� �� �����������
	//         ||||ADIE       - ���������� ���������� �� �����������
	//         |||||ADPS2     - ��������� ����������� (������ �� ����� ���������)
	//         ||||||ADPS1    -
	//         |||||||ADPS0   -
	//         ||||||||
	ADCSRA = 0b10000111;
}

inline void SetupADMUX(int src){
	// ������� 1
	//  REFS2  REFS1  REFS0  - ����������� �������� �������� ����������
	//    x      0      0    - VCC ������� ����������
	//    x      0      1    - ������� ���������� �� AREF
	//    0      1      0    - ���������� 1.1� ������� ����������
	//    0      1      1    - ���������������
	//    1      1      0    - ���������� 2.56� ������� ���������� ��� ������������
	//    1      1      1    - ���������� 2.56� ������� ���������� � ������������� �� ���� AREF

	// ������� 2
	//  MUX3 MUX2 MUX1 MUX0 -  ���� ���������� �� ������������� (� ���� ��������� ���)
	//    0    0    0    0  - PB5
	//    0    0    0    1  - PB2
	//    0    0    1    0  - PB4
	//    0    0    1    1  - PB3
	// ��������� ����� ��� ���������� (�� ������������)
	//    0    1    0    0  - PB4 PB4 x1
	//    0    1    0    1  - PB4 PB4 x20
	//    0    1    1    0  - PB4 PB3 x1
	//    0    1    1    1  - PB4 PB3 x20
	//    1    0    0    0  - PB5 PB5 x1
	//    1    0    0    1  - PB5 PB5 x20
	//    1    0    1    0  - PB5 PB2 x1
	//    1    0    1    1  - PB5 PB2 x20
	//    1    1    0    0  - Vbg
	//    1    1    0    1  - GND
	//    1    1    1    0  - N/A
	//    1    1    1    1  - ADC4

	//        REFS1           - ������� 1
	//        |REFS0          - ������� 1
	//        ||ADLAR         - ��� ���������� �� ������� ������ ����� ���������� � �������� (0 - 10 ���; 1 - 8 ���)
	//        |||REFS2        - ������� 1
	//        ||||MUX3        - ������� 2
	//        |||||MUX2       - ������� 2
	//        ||||||MUX1      - ������� 2
	//        |||||||MUX0     - ������� 2
	//        ||||||||
	ADMUX = 0b00100000 | (src & 0b00001111);
}
	
int ReadADC(int number_of_port){
	SetupADMUX(number_of_port);
	SetupADCSRA();
	ADCSRA |= (1 << ADSC);          // ������ ���������
	while(ADCSRA & (1 << ADSC));    // ���� ����� ��������
	int result = ADCH;              // ��������� ������ ���������� ����
	ADCSRA = 0b00000111;            // ��������� ���
	return result;
}

inline void SetupPins(){
  //         x              - ���� �������� �� ���������� �����
  //         |x             - 1 - OUTPUT; 0 - INPUT
  //         ||DDB5         - PB5
  //         |||DDB4        - PB4
  //         ||||DDB3       - PB3
  //         |||||DDB2      - PB2
  //         ||||||DDB1     - PB1
  //         |||||||DDB0    - PB0
  //         ||||||||
	DDRB = 0b00111010;
}

inline void SetupWatchdog(){
    // ������� 1 
	// WDP3  WDP2  WDP1  WDP0 - �������� �� ����� ������������ watchdog
	//  0      0     0     0  - 16ms
	//  0      1     0     0  - 25ms
	//  0      1     1     0  - 1s
	//  0      1     1     1  - 2s
	//  1      0     0     0  - 4s
	//  1      0     0     1  - 8s - maximum
	
	// ������� 2 
	// WDE  WDIE  - �������� �� ������ watchdog
	//  0    0    - none
	//  0    1    - interrupt mode
	//  1    0    - reset mode
	//  1    1    - interrupt reset mode

	//        WDIF            - �������������� 1 ����� ������� ������ ����������� (� ������ ����������)
	//        |WDIE           - ������� 2   ������ ������
	//        ||WDP3          - ������� 1
	//        |||WDCE         - ���������� ���������� ������� ����� WDE - 0
	//        ||||WDE         - ��������� watchdog
	//        |||||WDP2       - ������� 1
	//        ||||||WDP1      - ������� 1
	//        |||||||WDP0     - ������� 1
	//        ||||||||
	WDTCR = 0b01111000;
	
}

inline void blink(int ms){
	PORTB = 0b00010000;
	_delay_ms(ms);
	PORTB = 0b00000000;
	_delay_ms(ms);
}

int main(void)
{
	SetupPins();
	sei();
	clear();
	SetupWatchdog();
	ADCSRA = 0b00000111;
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	//blink(1000);
	//checkCounter(3);    //���������� ������ ��� ������������ 

	 // if (returnCounter() == 3){
		//blink(1000);
	  //}
	  
	//counterPlus();
	
	while (1) {
		sleep_mode();
    }
}

