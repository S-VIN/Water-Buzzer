# define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

const int n = 128;
//PB0 - сигнал с датчика влажности
//PB1 - питание датчика воды
//PB2 - сигнал с датчика света
//PB3 - питание датчика света
//PB4 - сигнал на пищалку
//PB5 - ресет пин

class eeprom{
	public:

	void clear(){
		for (int i =0; i <= n; i++){
			
		}
	}
	int searchZero(){
		for(int i = 0; i <= n; i++){
			if (EEPROM[i] == 0){
				return i;
			}
		}
		return 0;
	}
	int returnCounter(){
		int zero = searchZero();
		if (zero == 0){
			return EEPROM[n];
		}
		return EEPROM[zero - 1];
	}
	bool checkCounter(int maxi = 10){
		if (returnCounter() == maxi){
			if (searchZero() == n){
				int zero = searchZero();
				EEPROM[0] = 0;
				EEPROM[zero] = 1;
				} else {
				int zero = searchZero();
				EEPROM[zero + 1] = 0;
				EEPROM[zero] = 1;
			}
		}
	}
	void counterPlus(){
		int zero = searchZero();
		if (zero == 0){
			EEPROM[n]++;
			} else{
			EEPROM[zero - 1]++;
		}
	}
};

inline void SetupADCSRA(){
	//         ADEN           - Подать питание на АЦП
	//         |ADSC          - Запуск АЦП
	//         ||ADATE        - Оцифровка по срабатываению триггера ADTS[2:0]
	//         |||ADIF        - флаг прерывания от компаратора
	//         ||||ADIE       - разрешение прерывания от компаратора
	//         |||||ADPS2     - настройка пределителя (влияет на время измерения)
	//         ||||||ADPS1    -
	//         |||||||ADPS0   -
	//         ||||||||
	ADCSRA = 0b10000111;
}

inline void SetupADMUX(int src){
	// таблица 1
	//  REFS2  REFS1  REFS0  - настраивает источник опорного напряжения
	//    x      0      0    - VCC опорное напряжение
	//    x      0      1    - опорное напряжение на AREF
	//    0      1      0    - внутреннее 1.1В опорное напряжение
	//    0      1      1    - зарезервировано
	//    1      1      0    - внутреннее 2.56В опорное напряжение без конденсатора
	//    1      1      1    - внутреннее 2.56В опорное напряжение с конденсатором на пине AREF

	// таблица 2
	//  MUX3 MUX2 MUX1 MUX0 -  биты отвечающие за мультиплексор (к чему подключен АЦП)
	//    0    0    0    0  - PB5
	//    0    0    0    1  - PB2
	//    0    0    1    0  - PB4
	//    0    0    1    1  - PB3
	// следующие нужны для калибровки (не использовать)
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

	//        REFS1           - таблица 1
	//        |REFS0          - таблица 1
	//        ||ADLAR         - бит отвечающий за порядок записи битов результата в регистры (0 - 10 бит; 1 - 8 бит)
	//        |||REFS2        - таблица 1
	//        ||||MUX3        - таблица 2
	//        |||||MUX2       - таблица 2
	//        ||||||MUX1      - таблица 2
	//        |||||||MUX0     - таблица 2
	//        ||||||||
	ADMUX = 0b00100000 | (src & 0b00001111);
}
	
int ReadADC(int number_of_port){
	SetupADMUX(number_of_port);
	SetupADCSRA();
	ADCSRA |= (1 << ADSC);          // начали измерение
	while(ADCSRA & (1 << ADSC));    // ждем когда помереет
	int result = ADCH;              // отбросили шумные незначащие биты
	ADCSRA = 0b00000111;            // выключили АЦП
	return result;
}

inline void SetupPins(){
  //         x              - Биты отвечают за назначение пинов
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
    // таблица 1 
	// WDP3  WDP2  WDP1  WDP0 - отвечают за время срабатывания watchdog
	//  0      0     0     0  - 16ms
	//  0      1     0     0  - 25ms
	//  0      1     1     0  - 1s
	//  0      1     1     1  - 2s
	//  1      0     0     0  - 4s
	//  1      0     0     1  - 8s - maximum
	
	// таблица 2 
	// WDE  WDIE  - отвечают за режимы watchdog
	//  0    0    - none
	//  0    1    - interrupt mode
	//  1    0    - reset mode
	//  1    1    - interrupt reset mode

	//        WDIF            - устнавливается 1 когда вотчдог таймер срабатывает (в режиме прерываний)
	//        |WDIE           - таблица 2   меняет режимы
	//        ||WDP3          - таблица 1
	//        |||WDCE         - необходимо установить еденицу когда WDE - 0
	//        ||||WDE         - включение watchdog
	//        |||||WDP2       - таблица 1
	//        ||||||WDP1      - таблица 1
	//        |||||||WDP0     - таблица 1
	//        ||||||||
	WDTCR = 0b00001111;
}

inline void SetupSleep(){
	// таблица 1 
	// SM1   SM0
	//  0     0    - idle (работает все кроме ЦП)
	//  0     1    - ADC Noise Reduction (нужно чтоб помочь АЦП измерять точнее. отключает что-то что мешает АЦП)
	//  1     0    - Power-down (отключает все, кроме watchdog и обработчика прерываний. самый крутой режим)
	//  1     1    - reserved  (не лезь)
	
	
	//        BODS              - не поддерживается (0 по умолчанию)
	//        |PUD              - хз что, нет в даташите
	//        ||SE              - выключатель режима сна (установи 1 чтоб заснуть)
	//        |||SM1            - таблица 1
	//        ||||SM0           - таблица 1
	//        |||||BODSE        - не поддерживается (0 по умолчанию)
	//        ||||||ISC01       - что-то связанное с прерываниями (к делу не относится)
	//        |||||||ISC00      - что-то связанное с прерываниями (к делу не относится)
	//        ||||||||
	MCUCR = 0b00010000;
}

inline void Sleep(){
	//  вся вспомогательная информация в SetupSleep
	MCUCR |= (1 << SE);
}

inline void blink(){
	PORTB = 0b00010000;
	_delay_ms(1000);
	PORTB = 0b00000000;
	_delay_ms(1000);
}



int main(void)
{
	SetupPins();
	SetupWatchdog();
	SetupSleep();
	ReadADC(1);
  	blink();
	while (1) {
		Sleep();
    }
}

