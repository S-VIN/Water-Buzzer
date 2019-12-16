//PB0 - ������� �������������
//PB1 - ������� �������
//PB2 - ������� ������� �������
//PB3 - ������ � �������������
//PB4 - ������ ������� �������
//PB5 - ����� ���

# define F_CPU 1000000UL

#include "util.h"
//#include "uart.h"
#include "pinsFuncs.h"
#include "buzz.h"

const uint32_t sensorSurvey = 1200; //� �������� 

int interruptFlug = 0;

ISR (WDT_vect) {
	interruptFlug++;
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

inline void DoCode(){
	if ((ReadLight() > 100) && (ReadWater() < 100)){
		alarmSound();
	}
}

int main(void)
{
	int realSurvey = sensorSurvey / 8;
	SetupPins();
	SetupWatchdog();
	
	for(int i = 0; i <= 3; i++){
		tweet(5, 3);
		delay_ms(500);
	}
	
	for(int i = 0; i <= 60; i++){
		//uart_tx_init();
		//num_to_str(ReadWater(), 4);
		//uart_print("\r\n");
		if ((ReadLight() > 100) && (ReadWater() < 100)){
			alarmSound();
		}
		delay_ms(1000);
	}
	
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	while (1) {
		if (interruptFlug >= realSurvey){
			interruptFlug = 0;
			DoCode();
		}
		sleep_mode();
	}
}





