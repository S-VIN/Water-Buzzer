//# define F_CPU 1000000UL



#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

const size_t rBufSize = 128;
const size_t rBufOffs = 0;
//PB0 - сигнал с датчика влажности
//PB1 - питание датчика воды
//PB2 - сигнал с датчика света
//PB3 - питание датчика света
//PB4 - сигнал на пищалку
//PB5 - ресет пин


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
	//checkCounter(3);    //сбрасываем таймер при переполнении 

	 // if (returnCounter() == 3){
		//blink(1000);
	  //}
	  
	//counterPlus();
	
	while (1) {
		sleep_mode();
    }
}

