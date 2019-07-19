# define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

const int sensorSurvey = 10; //� �������� 
int interruptFlug = 0;

//PB0 - ������� �������������
//PB1 - ������� �������
//PB2 - ������� ������� �������
//PB3 - ������ � �������������
//PB4 - ������ ������� �������
//PB5 - ����� ���

inline void SetupPins(){
	//       x              - ���� �������� �� ���������� �����
	//       |x             - 1 - OUTPUT; 0 - INPUT
	//       ||DDB5         - PB5
	//       |||DDB4        - PB4
	//       ||||DDB3       - PB3
	//       |||||DDB2      - PB2
	//       ||||||DDB1     - PB1
	//       |||||||DDB0    - PB0
	//       ||||||||
	DDRB = 0b00100111;
	
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
	WDTCR = 0b01110001;
	sei();
}

inline void Buzz(int ms){
	PORTB = 0b00000010;
	_delay_ms(ms);
	PORTB = 0b00000000;
	_delay_ms(ms);
}


ISR (WDT_vect) {
	interruptFlug++;
}


inline void DoCode(){
	PORTB = 0b00000101;
	//if((ReadADC(3) > 500) && (ReadADC(2) > 400)){
		Buzz(1000);
	//}
	//PORTB = 0b00000000;
}

int main(void)
{
	int realSurvey = sensorSurvey / 8;
	SetupPins();
	SetupWatchdog();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	while (1) {
		if (interruptFlug >= realSurvey){
			
			interruptFlug = 0;
		}
		sleep_mode();
	}
}