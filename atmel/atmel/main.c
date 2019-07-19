# define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

const int n = 128;

//PB0 - ������ � ������� ���������
//PB1 - ������� ������� ����
//PB2 - ������ � ������� �����
//PB3 - ������� ������� �����
//PB4 - ������ �� �������
//PB5 - ����� ���

int Eread(int i){
	return eeprom_read_byte(i);
}

void Ewrite(int addr,  int inf){
	eeprom_write_byte( addr,  inf) ;
}

void clear(){
	for (int i =0; i <= n; i++){
		Ewrite(i, 0);
	}
}

int searchZero(){
	int inf = 0;
	for(int i = 0; i <= n; i++){
		if (Eread(i) == 0){
			return i;
		}
	}
	return 0;
}

int returnCounter(){
	int zero = searchZero();
	if (zero == 0){
		return Eread(n);
	}
	return Eread(zero - 1);
}

void checkCounter(int maxi){
	if (returnCounter() == maxi){
		if (searchZero() == n){
			int zero = searchZero();
			Ewrite(0, 0);
			Ewrite(zero, 1);
			} else {
			int zero = searchZero();
			Ewrite(zero + 1, 0);
			Ewrite(zero, 1);
		}
	}
}

void counterPlus(){
	int zero = searchZero();
	if (zero == 0){
		Ewrite(n, Eread(n) + 1);
		} else{
		Ewrite(zero - 1, Eread(zero - 1) + 1);
	}
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
	
	SetupWatchdog();
	ADCSRA = 0b00000111;
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	checkCounter(3);    //���������� ������ ��� ������������ 

	  if (returnCounter() == 3){
		blink(100000);
	  }
	  
	counterPlus();
	
	while (1) {
		sleep_mode();
    }
}

